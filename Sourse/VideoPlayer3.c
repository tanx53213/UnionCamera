#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include "camera.h"
#include <errno.h>

// 递归创建目录 (等效 mkdir -p)
static int mkdir_p(const char *path, mode_t mode)
{
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;

    for (p = tmp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = 0;
            if (mkdir(tmp, mode) != 0 && errno != EEXIST)
                return -1;
            *p = '/';
        }
    }
    if (mkdir(tmp, mode) != 0 && errno != EEXIST)
        return -1;
    return 0;
}

// 定义录像文件夹路径 (RECORD_PATH 已在 camera.h 中统一定义)
#define MAX_RECORDS 200

// TS_DEV, LCD_DEV, LCD_MAPSIZE 已在 camera.h 中定义

// lcd_fd, lcd_mp, camera_fd, n_buffers, buffers 已在 API_Camera2.c 中定义
// 此处通过 camera.h 的 extern 声明引用

extern int buttonCount;

// 异步线程 ID 与运行状态标志
static pthread_t backup_tid;
static bool backup_thread_active = false;
static pthread_t record_tid;
static bool record_thread_active = false;
static pthread_t guard_tid;
static bool guard_thread_active = false;

// 线程包装函数声明
static void *recording_thread(void *arg);
static void *guard_thread(void *arg);

/****************************************************************************************/

// 停止摄像头函数
void stop_camera(void)
{
    if (camera_fd >= 0)
    {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ioctl(camera_fd, VIDIOC_STREAMOFF, &type);
        close(camera_fd);
        camera_fd = -1;
    }
}

// 返回主界面函数
void return_to_main(void)
{
    // 先改变状态，通知各线程退出循环
    ProgramState prev_state = current_state;
    current_state = STATE_MAIN;

    switch (prev_state)
    {
        case STATE_BACKUP:
            printf("退出后视功能\n");
            if (backup_thread_active)
            {
                pthread_join(backup_tid, NULL);
                backup_thread_active = false;
            }
            stop_camera();
            break;
        case STATE_RECORD:
            printf("退出录像功能\n");
            if (record_thread_active)
            {
                pthread_join(record_tid, NULL);
                record_thread_active = false;
            }
            stop_camera();
            break;
        case STATE_HISTORY:
            printf("退出历史记录\n");
            break;
        case STATE_GUARD:
            printf("退出守护进程\n");
            if (guard_thread_active)
            {
                pthread_join(guard_tid, NULL);
                guard_thread_active = false;
            }
            stop_camera();
            break;
        default:
            break;
    }

    char main_ui_path[256] = {0};
    resolve_asset_path(main_ui_path, sizeof(main_ui_path), "runV1.bmp");
    Bmp_Decode(main_ui_path, lcd_mp);
    current_state = STATE_MAIN;
}

/****************************************************************************************/

// YUV转BMP（保留原有函数，同时作为yuyv_to_bmp3使用）
void yuyv_to_bmp(char *yuvbuf, const char *bmp_file)
{
    FILE *bmp_fp = fopen(bmp_file, "wb");
    if (!bmp_fp)
    {
        printf("创建BMP文件失败\n");
        return;
    }

    char header[54] = {0};
    fwrite(header, 1, 54, bmp_fp);
    fseek(bmp_fp, 54, SEEK_SET);

    char buf_line[640 * 3] = {0};

    for (int y = 479; y >= 0; y--)
    {
        for (int x = 0; x < 640; x += 2)
        {
            int j = y * 640 * 2 + x * 2;
            unsigned int rgb0 = pixel_yuv2rgb(yuvbuf[j+0], yuvbuf[j+1], yuvbuf[j+3]);
            unsigned int rgb1 = pixel_yuv2rgb(yuvbuf[j+2], yuvbuf[j+1], yuvbuf[j+3]);

            buf_line[x * 3]     = (rgb0) & 0xFF;
            buf_line[x * 3 + 1] = (rgb0 >> 8) & 0xFF;
            buf_line[x * 3 + 2] = (rgb0 >> 16) & 0xFF;
            buf_line[(x+1) * 3]     = (rgb1) & 0xFF;
            buf_line[(x+1) * 3 + 1] = (rgb1 >> 8) & 0xFF;
            buf_line[(x+1) * 3 + 2] = (rgb1 >> 16) & 0xFF;
        }
        fwrite(buf_line, 1, 640 * 3, bmp_fp);
    }

    fclose(bmp_fp);
}

// yuyv_to_bmp3 直接复用 yuyv_to_bmp
void yuyv_to_bmp3(char *yuvbuf, const char *bmp_file)
{
    yuyv_to_bmp(yuvbuf, bmp_file);
}

/****************************************************************************************/

// 录像功能（只保留一份，合并了子目录+bmp保存的新版逻辑）
void record_video(void)
{
    char timestamp[32];
    char filepath[256];
    char dir_path[256];
    time_t now;
    struct tm *timeinfo;
    bool first_frame = true;

    // 确保摄像头未打开，避免资源泄漏
    if (camera_fd >= 0)
        stop_camera();

    open_device();
    init_device();
    start_capturing();

    // 创建录像根目录
    if (mkdir_p(RECORD_PATH, 0777) != 0)
    {
        printf("Failed to create record directory: %s\n", RECORD_PATH);
        stop_camera();
        return;
    }

    // 以时间戳创建子目录
    time(&now);
    timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", timeinfo);
    snprintf(dir_path, sizeof(dir_path), "%s/%s", RECORD_PATH, timestamp);
    if (mkdir(dir_path, 0777) != 0)
    {
        printf("Failed to create record subdirectory: %s\n", dir_path);
        stop_camera();
        return;
    }

    // yuv原始数据文件路径
    snprintf(filepath, sizeof(filepath), "%s/%s.yuv", dir_path, timestamp);

    FILE *fp = fopen(filepath, "wb");
    if (!fp)
    {
        printf("Failed to create record file\n");
        stop_camera();
        return;
    }

    int frame_count = 0;
    while (frame_count < 150 && (current_state == STATE_RECORD || current_state == STATE_GUARD))
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        fd_set fds;
        struct timeval tv;
        FD_ZERO(&fds);
        FD_SET(camera_fd, &fds);
        tv.tv_sec  = 2;
        tv.tv_usec = 0;

        int r = select(camera_fd + 1, &fds, NULL, NULL, &tv);
        if (r == -1)
        {
            if (errno == EINTR) continue;
            break;
        }
        if (r == 0)
        {
            printf("select timeout\n");
            break;
        }

        if (-1 == ioctl(camera_fd, VIDIOC_DQBUF, &buf))
            break;

        // 第一帧显示到LCD
        if (first_frame)
        {
            yuyv_to_lcd(buffers[buf.index].start);
            first_frame = false;
            printf("显示第一帧\n");
        }

        // 每帧保存为BMP
        char bmp_file[512];
        snprintf(bmp_file, sizeof(bmp_file), "%s/frame_%03d.bmp", dir_path, frame_count);
        yuyv_to_bmp(buffers[buf.index].start, bmp_file);

        // 写入yuv原始数据
        fwrite(buffers[buf.index].start, 1, buf.bytesused, fp);

        if (-1 == ioctl(camera_fd, VIDIOC_QBUF, &buf))
            break;

        frame_count++;
        printf("\r录制进度: %d%%", (frame_count * 100) / 150);
        fflush(stdout);
    }

    printf("\n录制完成\n");
    fclose(fp);
    stop_camera();
}

/****************************************************************************************/

// 显示历史记录
void show_history(void)
{
    DIR *dir;
    struct dirent *ent;
    char filepath[256];

    dir = opendir(RECORD_PATH);
    if (dir == NULL)
    {
        printf("No history records\n");
        return;
    }

    while ((ent = readdir(dir)) != NULL)
    {
        if (ent->d_name[0] == '.') continue;

        snprintf(filepath, sizeof(filepath), "%s/%s", RECORD_PATH, ent->d_name);
        FILE *fp = fopen(filepath, "rb");
        if (fp)
        {
            char *buffer = malloc(640 * 480 * 2);
            if (buffer)
            {
                size_t bytes_read = fread(buffer, 1, 640 * 480 * 2, fp);
                if (bytes_read > 0)
                    yuyv_to_lcd(buffer);
                free(buffer);
            }
            fclose(fp);
            break;
        }
    }
    closedir(dir);
}

/****************************************************************************************/

// 循环录像功能
void circular_recording(void)
{
    while (current_state == STATE_GUARD)
    {
        DIR *dir = opendir(RECORD_PATH);
        int count = 0;
        struct dirent *ent;

        if (dir)
        {
            while ((ent = readdir(dir)) != NULL)
            {
                if (ent->d_name[0] != '.') count++;
            }
            closedir(dir);
        }

        if (count >= MAX_RECORDS)
        {
            dir = opendir(RECORD_PATH);
            time_t oldest_time = time(NULL);
            char oldest_file[256] = {0};

            if (dir)
            {
                while ((ent = readdir(dir)) != NULL)
                {
                    if (ent->d_name[0] == '.') continue;

                    char filepath[512];
                    struct stat st;
                    snprintf(filepath, sizeof(filepath), "%s/%s", RECORD_PATH, ent->d_name);

                    if (stat(filepath, &st) == 0)
                    {
                        if (st.st_mtime < oldest_time)
                        {
                            oldest_time = st.st_mtime;
                            strcpy(oldest_file, filepath);
                        }
                    }
                }
                closedir(dir);

                if (oldest_file[0])
                {
                    // 安全删除过期录像文件/目录
                    DIR *d = opendir(oldest_file);
                    if (d)
                    {
                        // 是目录: 先删除内部文件再删除目录
                        struct dirent *de;
                        char sub[512];
                        while ((de = readdir(d)) != NULL)
                        {
                            if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
                                continue;
                            snprintf(sub, sizeof(sub), "%s/%s", oldest_file, de->d_name);
                            remove(sub);
                        }
                        closedir(d);
                        rmdir(oldest_file);
                    }
                    else
                    {
                        remove(oldest_file);
                    }
                    printf("过期录像: %s 已清理\n", oldest_file);
                }
            }
        }

        record_video();
    }
}

// 录像线程包装 (非阻塞)
static void *recording_thread(void *arg)
{
    (void)arg;
    record_video();
    if (current_state == STATE_RECORD)
        current_state = STATE_MAIN;
    return NULL;
}

// 守护进程线程包装 (非阻塞)
static void *guard_thread(void *arg)
{
    (void)arg;
    circular_recording();
    return NULL;
}

/****************************************************************************************/

// 按钮按下处理
void handleButtonPress(Button *button, int buttonIndex)
{
    if (button == NULL || buttonIndex < 0 || buttonIndex >= buttonCount)
    {
        printf("无效的按钮索引\n");
        return;
    }

    switch (buttonIndex)
    {
        case 0:  // 后视
            if (current_state == STATE_MAIN)
            {
                printf("进入后视功能\n");
                current_state = STATE_BACKUP;
                open_device();
                init_device();
                start_capturing();
                if (pthread_create(&backup_tid, NULL, backup_camera_thread, NULL) != 0)
                {
                    fprintf(stderr, "创建后视线程失败\n");
                    stop_camera();
                    current_state = STATE_MAIN;
                }
                else
                {
                    backup_thread_active = true;
                }
            }
            break;

        case 1:  // 录像
            if (current_state == STATE_MAIN)
            {
                printf("进入录像功能\n");
                current_state = STATE_RECORD;
                if (pthread_create(&record_tid, NULL, recording_thread, NULL) != 0)
                {
                    fprintf(stderr, "创建录像线程失败\n");
                    current_state = STATE_MAIN;
                }
                else
                {
                    record_thread_active = true;
                }
            }
            break;

        case 2:  // 历史
            if (current_state == STATE_MAIN)
            {
                printf("进入历史记录\n");
                current_state = STATE_HISTORY;
                show_history();
            }
            break;

        case 3:  // 返回
            if (current_state != STATE_MAIN)
                return_to_main();
            break;

        case 4:  // 守护进程
            if (current_state == STATE_MAIN)
            {
                printf("进入守护进程\n");
                current_state = STATE_GUARD;
                if (pthread_create(&guard_tid, NULL, guard_thread, NULL) != 0)
                {
                    fprintf(stderr, "创建守护线程失败\n");
                    current_state = STATE_MAIN;
                }
                else
                {
                    guard_thread_active = true;
                }
            }
            break;

        case 5:  // 电源键
            if (current_state == STATE_MAIN)
            {
                printf("程序退出\n");
                stop_camera();
                exit(0);
            }
            else
            {
                return_to_main();
            }
            break;
    }
}

/****************************************************************************************/

// 触摸事件处理
void handleTouchEvent(ts_pix *event, Button *buttons, int buttonCount)
{
    if (!event->isPressed)
        return;

    for (int i = 0; i < buttonCount; i++)
    {
        if (isPointInButton(event->pix_x, event->pix_y, &buttons[i]))
        {
            handleButtonPress(&buttons[i], i);
            break;
        }
    }
}