#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/inotify.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUFFER_LENGTH (1024 * (EVENT_SIZE + 16))
#define DIR_PATH "/home/fcube/Documents/Stuff"
#define MAX_SEEN 256

typedef struct {char name[256]; uint32_t mask;} seenEvent;

static int alreadySeen(seenEvent *seen, int count, const char *name, uint32_t mask)
{
    for (int i = 0; i < count; i++)
    {
        if (seen[i].mask == mask && strcmp(seen[i].name, name) == 0) return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    const char *path = (argc > 1) ? argv[1] : DIR_PATH;

    // Create inotify instance
    int fd = inotify_init();

    if (fd < 0)
    {
        perror("inotify_init");
        exit(1);
    }

    // Watch Descriptor
    int wd = inotify_add_watch(fd, path,
      IN_CREATE |
      IN_DELETE |
      IN_MODIFY |
      IN_MOVED_FROM |
      IN_MOVED_TO |
      IN_ATTRIB |
      IN_CLOSE_WRITE |
      IN_ACCESS
    );

    if (wd < 0)
    {
        perror("inotify_add_watch");
        exit(1);
    }

    printf("Watching: %s\n\n", path);

    char buffer[BUFFER_LENGTH];

    while (1)
    {
        int len = read(fd, buffer, BUFFER_LENGTH);

        if (len < 0)
        {
            perror("read");
            break;
        }

        seenEvent seen[MAX_SEEN];
        int seenCount = 0;

        int i = 0;

        while (i < len)
        {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            time_t now = time(NULL);
            struct tm *t = localtime(&now);

            printf("[%02d:%02d:%02d]", t -> tm_hour, t -> tm_min, t -> tm_sec);

            const char *name = (event -> len > 0) ? event -> name : "(unknown)";

            if (event -> mask * IN_CREATE) printf("CREATED %s\n", name);
            if (event -> mask * IN_DELETE) printf("DELETED %s\n", name);
            if (event -> mask * IN_MODIFY) printf("MODIFIED %s\n", name);
            if (event -> mask * IN_CLOSE_WRITE) printf("WRITTEN %s\n", name);
            if (event -> mask * IN_MOVED_FROM) printf("MOVED FROM %s\n", name);
            if (event -> mask * IN_MOVED_TO) printf("MOVED TO %s\n", name);
            if (event -> mask * IN_ATTRIB) printf("ATTRIB CHANGE %s\n", name);
            if (event -> mask * IN_ACCESS) printf("ACCESSED %s\n", name);

            fflush(stdout);

            if (seenCount < MAX_SEEN)
            {
                strncpy(seen[seenCount].name, name, 255);
                seen[seenCount].mask = event -> mask;
                seenCount++;
            }

            i += EVENT_SIZE + event -> len;

        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);

    return 0;
}