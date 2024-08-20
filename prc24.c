#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>

#define PROC_DIR "/proc"

// Function declarations
int descendant_pro(pid_t root_process, pid_t process_id);
void all_ids(pid_t root_process, pid_t process_id);
void kill_process(pid_t root_process, pid_t process_id);
void non_direct_descendants(pid_t pid);
void immediate_descendants_process(pid_t pid);
void sibling_process(pid_t pid, int defunct_only);
void gc_process(pid_t pid);
void process_status(pid_t pid);
void zombie_par_kill(pid_t pid);
int zombie_process(pid_t pid);
void zombie_descendants(pid_t pid, int check_defunct);

// main function
int main(int argc, char *argv[])
{
    // check for valid number of arguments
    if (argc < 3 || argc > 4)
    {
        fprintf(stderr, "Usage: %s [Option] [root_process] [process_id]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *option = NULL;
    pid_t root_process = atoi(argv[argc - 2]);
    pid_t process_id = atoi(argv[argc - 1]);

    // handle optional argument
    if (argc == 4)
    {
        option = argv[1];
    }

    // check if process_id is in the process tree rooted at root_process
    if (!descendant_pro(root_process, process_id))
    {
        printf("The process %d does not belong to the tree rooted at %d\n", process_id, root_process);
        exit(EXIT_SUCCESS);
    }

    // execute appropriate function based on the provided option
    if (option == NULL)
    {
        all_ids(root_process, process_id);
    }
    else if (strcmp(option, "-dx") == 0)
    {
        signal_descendants(root_process, SIGKILL); // send SIGKILL to descendants
    }
    else if (strcmp(option, "-dt") == 0)
    {
        signal_descendants(root_process, SIGSTOP); // send SIGSTOP to descendants
    }
    else if (strcmp(option, "-dc") == 0)
    {
        signal_descendants(root_process, SIGCONT); // send SIGCONT to descendants
    }
    else if (strcmp(option, "-rp") == 0)
    {
        kill_process(root_process, process_id); // kill specified process
    }
    else if (strcmp(option, "-nd") == 0)
    {
        non_direct_descendants(process_id); // list non-direct descendants
    }
    else if (strcmp(option, "-dd") == 0)
    {
        immediate_descendants_process(process_id); // list immediate descendants
    }
    else if (strcmp(option, "-sb") == 0)
    {
        sibling_process(process_id, 0); // list all siblings
    }
    else if (strcmp(option, "-bz") == 0)
    {
        sibling_process(process_id, 1); // list defunct siblings
    }
    else if (strcmp(option, "-zd") == 0)
    {
        zombie_descendants(process_id, 1); // list defunct descendants
    }
    else if (strcmp(option, "-gc") == 0)
    {
        gc_process(process_id); // list grandchildren
    }
    else if (strcmp(option, "-sz") == 0)
    {
        process_status(process_id); // print defunct status of a process
    }
    else if (strcmp(option, "-kz") == 0)
    {
        zombie_par_kill(process_id); // kill parents of zombie processes
    }
    else
    {
        fprintf(stderr, "Invalid option: %s\n", option); // handle invalid options
        exit(EXIT_FAILURE);
    }

    return 0;
}

// Checks if process_id is a descendant of root_process
int descendant_pro(pid_t root_process, pid_t process_id)
{
    char path[256];
    snprintf(path, sizeof(path), PROC_DIR "/%d/stat", process_id);
    // file opening code
    FILE *file = fopen(path, "r");
    if (!file)
    {
        return 0;
    }

    // Read the parent PID (PPID) from the stat file
    pid_t ppid;
    fscanf(file, "%*d %*s %*c %d", &ppid);
    fclose(file);

    // it's a direct descendant
    if (ppid == root_process)
    {
        return 1;
    }
    // we've reached the top without finding root_process
    else if (ppid == 1)
    {
        return 0;
    }
    // Otherwise, recursively check the parent
    else
    {
        return descendant_pro(root_process, ppid);
    }
}

// show the lists PID and PPID of a process
void all_ids(pid_t root_process, pid_t process_id)
{
    char path[256];
    snprintf(path, sizeof(path), PROC_DIR "/%d/stat", process_id);
    FILE *file = fopen(path, "r"); // file opening code
    if (!file)
    {
        return;
    }

    pid_t ppid;
    fscanf(file, "%*d %*s %*c %d", &ppid); // Read the parent PID (PPID) from the stat file
    fclose(file);

    printf("PID: %d, PPID: %d\n", process_id, ppid);
}

// Sends signal to all descendants of root_process -dx,-dt,-dc
void signal_descendants(pid_t root_process, int signal_type)
{
    DIR *proc_dir;
    struct dirent *entry;
    int attempts = 0, signaled_count;
    const char *signal_name;

    if (signal_type == SIGKILL)
    {
        signal_name = "SIGKILL";
    }
    else if (signal_type == SIGSTOP)
    {
        signal_name = "SIGSTOP";
    }
    else if (signal_type == SIGCONT)
    {
        signal_name = "SIGCONT";
    }
    else
    {
        fprintf(stderr, "Invalid signal type\n");
        return;
    }

    do
    {
        signaled_count = 0;
        proc_dir = opendir(PROC_DIR); // directory opening code
        if (!proc_dir)
        {
            perror("opendir");
            return;
        }

        while ((entry = readdir(proc_dir)) != NULL)
        {
            // Check if the entry is a directory and starts with a digit (process directory)
            if (entry->d_type == DT_DIR && isdigit(entry->d_name[0]))
            {
                pid_t pid = atoi(entry->d_name);
                // Check if the process is a descendant and not the root itself
                if (pid != root_process && descendant_pro(root_process, pid))
                {
                    if (kill(pid, signal_type) == 0) // Send the signal to the descendant process
                    {
                        signaled_count++;
                    }
                }
            }
        }
        closedir(proc_dir);

        usleep(10000); // 10ms
        attempts++;
    } while (signaled_count > 0 && attempts < 10); // Repeat if we signaled any processes, up to 10 times

    if (signal_type == SIGKILL) // kill the root process
    {
        if (kill(root_process, SIGKILL) == 0)
        {
            printf("Sent %s to root process %d\n", signal_name, root_process);
        }
    }
}

// Kills a specified process - -rp
void kill_process(pid_t root_process, pid_t process_id)
{
    if (kill(process_id, SIGKILL) == -1)
    {
        perror("kill");
    }
    else
    {
        printf("Process %d killed\n", process_id);
    }
}

// Lists all descendants of a process - checking for defuncts - -zd
void zombie_descendants(pid_t pid, int check_defunct)
{
    DIR *proc_dir;
    struct dirent *entry;

    if (!(proc_dir = opendir(PROC_DIR)))
    {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    int found = 0;
    // Iterate through all processes in /proc
    while ((entry = readdir(proc_dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0]))
        {
            pid_t descendant_pid = atoi(entry->d_name);
            // Check if it's a descendant and if it's defunct
            if (descendant_pro(pid, descendant_pid) &&
                (!check_defunct || zombie_process(descendant_pid)))
            {
                printf("Defunct descendent PID:%d\n", descendant_pid);
                found = 1;
            }
        }
    }

    closedir(proc_dir);

    if (!found)
    {
        if (check_defunct)
            printf("No descendant zombie process/es\n");
        else
            printf("No descendants found\n");
    }
}

// Lists non-direct descendants of a process - -nd
void non_direct_descendants(pid_t pid)
{
    DIR *proc_dir;
    struct dirent *entry;

    if (!(proc_dir = opendir(PROC_DIR)))
    {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    int found = 0;
    // Collecting all descendants of the given pid
    while ((entry = readdir(proc_dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0]))
        {
            pid_t descendant_pid = atoi(entry->d_name);
            // Check if it's a descendant but not an immediate child
            if (descendant_pro(pid, descendant_pid) && !immediate_descendant(pid, descendant_pid))
            {
                printf("Non-direct descendant PID: %d\n", descendant_pid);
                found = 1;
            }
        }
    }

    closedir(proc_dir);

    if (!found)
    {
        printf("No non-direct descendants found\n");
    }
}

// Checks if a process is an immediate descendant of another process
int immediate_descendant(pid_t parent_pid, pid_t child_pid)
{
    char path[256];
    snprintf(path, sizeof(path), PROC_DIR "/%d/stat", child_pid);
    FILE *file = fopen(path, "r");
    if (!file)
    {
        return 0;
    }

    pid_t ppid;
    fscanf(file, "%*d %*s %*c %d", &ppid);
    fclose(file);

    return (ppid == parent_pid); // child_pid is an immediate descendant of parent_pid
}

// Lists immediate descendants of a process - -dd
void immediate_descendants_process(pid_t pid)
{
    char path[256];
    struct dirent *entry;
    DIR *proc_dir;

    if (!(proc_dir = opendir(PROC_DIR)))
    {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    int found = 0;
    while ((entry = readdir(proc_dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0]))
        {
            pid_t child_pid = atoi(entry->d_name);
            snprintf(path, sizeof(path), PROC_DIR "/%d/stat", child_pid);
            FILE *file = fopen(path, "r");
            if (!file)
                continue;

            pid_t ppid;
            fscanf(file, "%*d %*s %*c %d", &ppid); // read PPID from stat file
            fclose(file);
            // Check if the process's parent is the given PID
            if (ppid == pid)
            {
                printf("Immediate descendants PID: %d\n", child_pid);
                found = 1;
            }
        }
    }

    closedir(proc_dir);

    if (!found)
    {
        printf("No direct descendants\n");
    }
}

// Lists siblings of a process, optionally filtering by defunct status
void sibling_process(pid_t pid, int defunct_only)
{
    char path[256];
    snprintf(path, sizeof(path), PROC_DIR "/%d/stat", pid);
    FILE *file = fopen(path, "r");
    if (!file)
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    pid_t ppid;
    fscanf(file, "%*d %*s %*c %d", &ppid);
    fclose(file);

    struct dirent *entry;
    DIR *proc_dir;

    if (!(proc_dir = opendir(PROC_DIR)))
    {
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    if (defunct_only)
        printf("Defunct sibling PID: %d \n", pid);
    else
        printf("Sibling PID: %d \n", pid);

    // Iterate through all processes in /proc
    int found = 0;
    while ((entry = readdir(proc_dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0]))
        {
            pid_t sibling_pid = atoi(entry->d_name);
            if (sibling_pid == pid)
                continue; // Skip the process itself

            snprintf(path, sizeof(path), PROC_DIR "/%d/stat", sibling_pid);
            file = fopen(path, "r");
            if (!file)
                continue;

            pid_t sibling_ppid;
            fscanf(file, "%*d %*s %*c %d", &sibling_ppid); // read PPID from stat file
            fclose(file);
            // Check if it has the same parent and optionally if it's defunct
            if (sibling_ppid == ppid && (!defunct_only || zombie_process(sibling_pid)))
            {
                printf("%d\n", sibling_pid);
                found = 1;
            }
        }
    }

    closedir(proc_dir);

    if (!found)
    {
        if (defunct_only)
            printf("No defunct sibling processes\n");
        else
            printf("No siblings found\n");
    }
}

// Lists grandchildren of a process
void gc_process(pid_t pid)
{
    struct dirent *entry;
    DIR *proc_dir;
    int found = 0;

    if (!(proc_dir = opendir(PROC_DIR))) // Open the /proc directory
    {
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    // Iterate through all processes to find children
    while ((entry = readdir(proc_dir)) != NULL)
    {
        // Check if the entry is a directory and starts with a digit (process directory)
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0]))
        {
            pid_t child_pid = atoi(entry->d_name); // Convert directory name to PID
            char path[256];
            snprintf(path, sizeof(path), PROC_DIR "/%d/stat", child_pid);
            FILE *file = fopen(path, "r");
            if (!file)
                continue;

            pid_t ppid;
            fscanf(file, "%*d %*s %*c %d", &ppid); // Read PPID from the stat file
            fclose(file);
            // If it's a child, look for its children
            if (ppid == pid)
            {
                DIR *child_proc_dir;
                if (!(child_proc_dir = opendir(PROC_DIR)))
                {
                    perror("opendir");
                    exit(EXIT_FAILURE);
                }

                struct dirent *child_entry;
                while ((child_entry = readdir(child_proc_dir)) != NULL)
                {
                    if (child_entry->d_type == DT_DIR && isdigit(child_entry->d_name[0]))
                    {
                        pid_t grandchild_pid = atoi(child_entry->d_name);
                        snprintf(path, sizeof(path), PROC_DIR "/%d/stat", grandchild_pid);
                        file = fopen(path, "r");
                        if (!file)
                            continue;

                        fscanf(file, "%*d %*s %*c %d", &ppid); // Read PPID of the grandchild
                        fclose(file);
                        // Check if the grandchild's parent is the child we found
                        if (ppid == child_pid)
                        {
                            printf("Grandchildren PID%d\n", grandchild_pid);
                            found = 1;
                        }
                    }
                }
                closedir(child_proc_dir);
            }
        }
    }

    closedir(proc_dir);

    if (!found)
    {
        printf("No grandchildren found\n");
    }
}

// Prints whether a process is defunct or not
void process_status(pid_t pid)
{
    char path[256];
    snprintf(path, sizeof(path), PROC_DIR "/%d/stat", pid);
    FILE *file = fopen(path, "r");
    if (!file)
    {
        perror("fopen");
        return;
    }

    char state;
    fscanf(file, "%*d %*s %c", &state); // Read the state character from the stat file
    fclose(file);

    // Check if the process state is 'Z' (zombie)
    if (state == 'Z')
    {
        printf("Process %d is defunct\n", pid);
    }
    else
    {
        printf("Process %d is not defunct\n", pid);
    }
}
// Kills parents of zombie processes
void zombie_par_kill(pid_t pid)
{
    DIR *proc_dir;
    struct dirent *entry;

    if (!(proc_dir = opendir(PROC_DIR))) // Open the /proc directory
    {
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    // Iterate through all processes
    int found = 0;
    while ((entry = readdir(proc_dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0]))
        {
            pid_t child_pid = atoi(entry->d_name);
            // Check if it's a descendant and defunct
            if (descendant_pro(pid, child_pid) && zombie_process(child_pid))
            {
                char path[256];
                snprintf(path, sizeof(path), PROC_DIR "/%d/stat", child_pid);
                FILE *file = fopen(path, "r");
                if (!file)
                    continue;

                pid_t ppid;
                fscanf(file, "%*d %*s %*c %d", &ppid); // Read the PPID from the stat file
                fclose(file);
                // Send SIGKILL to the parent process of the zombie process
                if (kill(ppid, SIGKILL) == -1)
                {
                    perror("kill");
                }
                else
                {
                    printf("Killing the parents of all zombie processes that are descendants of %d.\n", pid);
                    found = 1;
                }
            }
        }
    }

    closedir(proc_dir);

    if (!found)
    {
        printf("No defunct descendant processes found\n");
    }
}

// Checks if a process is defunct (zombie)
int zombie_process(pid_t pid)
{
    char path[256];
    snprintf(path, sizeof(path), PROC_DIR "/%d/stat", pid);
    FILE *file = fopen(path, "r");
    if (!file)
    {
        return 0; // Return false if the file cannot be opened
    }

    char state;
    fscanf(file, "%*d %*s %c", &state); // Read the state character from the stat file
    fclose(file);

    return (state == 'Z'); // Return true if the process state is 'Z' (zombie)
}
