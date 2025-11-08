/*
 * Hi i am Tridib,A student of class 10 , Aspiring Full-Stack + AI Developer & Tech Entrepreneur !...
 * This is my phonebook  project
 * 
 * i built this project after understanding the fundamenatals of c language 
 * phonebook_pro.c
 *
 * polished, visually rich Phonebook CLI
 * - Dynamic/resizable memory phonebook
 * - Add / Search / View all / Delete contacts  [Based on users purpose]
 * - Binary save/load (.dat) with animated progress bars [For better experience]
 * - Password masking 
 * - Password strength check + confirmation
 * - Free-text gender & religion fields
 * - Authentication by ID + password before showing protected details
 * - ANSI colorized output, boxed layouts and human-friendly prompts [for beeter user experience]
 *
 * Build:
 *   gcc Phone_Book_Pro.c -o Phone_Book_Pro
 * Run:
 *   ./Phone_Book_Pro
 *
 * Security note:
 * - This demo stores passwords plaintext in memory and the binary file. 
 * 
 * i hope you would like my project
 * Leave a star if you like it
 * 
 * Thank you very much for your time !...
 * 
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <errno.h>
#include <limits.h>

#ifdef _WIN32
  #include <conio.h>
  #include <windows.h>
#else
  #include <termios.h>
  #include <unistd.h>
#endif

/* ----------------------------- ANSI COLORS ------------------------------ */
#define COL_RESET   "\x1b[0m"
#define COL_BOLD    "\x1b[1m"
#define COL_FAINT   "\x1b[2m"
#define COL_RED     "\x1b[31m"
#define COL_GREEN   "\x1b[32m"
#define COL_YELLOW  "\x1b[33m"
#define COL_BLUE    "\x1b[34m"
#define COL_CYAN    "\x1b[36m"
#define COL_MAGENTA "\x1b[35m"
#define COL_WHITE   "\x1b[37m"

/* ----------------------------- DATA STRUCTURES ------------------------- */

typedef struct job_role {
    char role_name[100];
    int experience_years;
    char company[100];
    float salary;
} job_role;

typedef struct address {
    int home_no;
    char neighbourhood[100];
    char road[100];
    char city[100];
    char state[100];
    char country[100];
} address;

typedef struct details {
    address place;
    int age;
    int id_no;
    char religion[64];   /* free-text */
    char gender[64];     /* free-text */
} details;

typedef struct person {
    details detail;
    job_role job;
    char name[100];
    long long ph_no;     /* numeric storage for phone without + */
    char gmail[256];
    char password[64];   /* plaintext here for demo */
} person;

/* ----------------------------- PHONEBOOK (DYN) ------------------------- */

typedef struct phonebook {
    person *data;
    size_t size;
    size_t capacity;
} phonebook;

static const char *DATA_FILE = "phonebook.dat";

/* ----------------------------- UTIL - DECLARATIONS --------------------- */

static void enable_ansi_on_windows(void);
static void clear_stdin(void);
static void fgets_trim(char *buf, size_t n);
static int str_case_equal(const char *a, const char *b);
static int str_case_prefix(const char *prefix, const char *s);
static void get_password_masked(char *buf, size_t bufsz, const char *prompt);
static int validate_phone_str(const char *s);
static int validate_email(const char *s);
static int password_strength_ok(const char *s);
static void msleep(unsigned int ms);

/* ----------------------------- PHONEBOOK API --------------------------- */

static void pb_init(phonebook *pb);
static void pb_free(phonebook *pb);
static void pb_ensure_capacity(phonebook *pb);
static void pb_add(phonebook *pb, const person *p);
static void pb_delete_at(phonebook *pb, size_t idx);

/* ----------------------------- SAVE / LOAD ---------------------------- */

static int save_to_file(phonebook *pb);
static int load_from_file(phonebook *pb);
static void show_progress_bar(const char *label, unsigned int duration_ms);

/* ----------------------------- UI HELPERS ----------------------------- */

static void print_boxed_title(const char *title);
static void print_divider(void);
static void print_centered_line(const char *s);
static void print_table_header(void);
static void print_trunc(const char *s, int width);

/* ----------------------------- CORE FEATURES -------------------------- */

static int authenticate(phonebook *pb, int id, const char *password);
static void add_contact(phonebook *pb);
static void view_all_contacts(phonebook *pb);
static int show_details_with_auth(phonebook *pb, size_t idx);
static void search_contact(phonebook *pb);
static void delete_contact(phonebook *pb);

/* ----------------------------- IMPLEMENTATION ------------------------- */

/* Enable ANSI escapes on Windows (best-effort) */
static void enable_ansi_on_windows(void) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            mode |= 0x0004; /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */
            SetConsoleMode(hOut, mode);
        }
    }
#endif
}

/* Clear leftover characters from stdin */
static void clear_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

/* Safe fgets and trim newline */
static void fgets_trim(char *buf, size_t n) {
    if (!fgets(buf, (int)n, stdin)) { buf[0] = '\0'; return; }
    size_t L = strlen(buf);
    if (L && buf[L-1] == '\n') buf[L-1] = '\0';
}

/* Full case-insensitive equality */
static int str_case_equal(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return (tolower((unsigned char)*a) == tolower((unsigned char)*b));
}

/* Prefix match case-insensitive */
static int str_case_prefix(const char *prefix, const char *s) {
    while (*prefix && *s) {
        if (tolower((unsigned char)*prefix) != tolower((unsigned char)*s)) return 0;
        prefix++; s++;
    }
    return 1;
}

/* Cross-platform password masking input */
static void get_password_masked(char *buf, size_t bufsz, const char *prompt) {
    printf("%s", prompt);
    fflush(stdout);

#ifdef _WIN32
    size_t i = 0;
    int ch;
    while ((ch = _getch()) != '\r' && ch != '\n') {
        if (ch == '\b' || ch == 127) {
            if (i) { i--; printf("\b \b"); }
        } else if (i + 1 < bufsz && i < 63) { /* Add maximum length check */
            buf[i++] = (char)ch;
            printf("*");
        } else if (i + 1 >= bufsz || i >= 63) {
            printf("\a"); /* Alert user that buffer is full */
        }
    }
    buf[i] = '\0';
    printf("\n");
#else
    struct termios oldt, newt;
    if (tcgetattr(STDIN_FILENO, &oldt) != 0) {
        /* fallback */
        fgets_trim(buf, bufsz);
        return;
    }
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    size_t i = 0;
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        if (ch == '\b' || ch == 127) {
            if (i) { i--; printf("\b \b"); }
        } else if (i + 1 < bufsz && i < 63) {
            buf[i++] = (char)ch;
            printf("*");
        } else if (i + 1 >= bufsz || i >= 63) {
            printf("\a"); /* Alert user that buffer is full */
        }
    }
    buf[i] = '\0';
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
#endif
}

/* Very small sleep for animation */
static void msleep(unsigned int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

/* Phone validation: digits allowed, optional leading +, length checks */
static int validate_phone_str(const char *s) {
    size_t L = strlen(s);
    if (L == 0) return 0;
    size_t digits = 0;
    for (size_t i = 0; i < L; i++) {
        if (s[i] == '+') continue;
        if (!isdigit((unsigned char)s[i])) return 0;
        digits++;
    }
    return digits >= 6 && digits <= 15;
}

/* Very simple email sanity check */
static int validate_string_length(const char *s, size_t min_len, size_t max_len) {
    size_t len = strlen(s);
    return len >= min_len && len <= max_len;
}

static int validate_numeric_input(const char *s, long long min_val, long long max_val) {
    char *endptr;
    long long val = strtoll(s, &endptr, 10);
    if (*endptr != '\0' || errno == ERANGE) return 0;
    return val >= min_val && val <= max_val;
}

static int validate_email(const char *s) {
    if (!validate_string_length(s, 5, 255)) return 0;
    const char *at = strchr(s, '@');
    if (!at) return 0;
    const char *dot = strrchr(at, '.');
    if (!dot) return 0;
    if (at == s) return 0;
    if (dot - at < 2) return 0;
    return 1;
}

/* Password strength: length >=8, contains lower, upper, digit, special */
static int password_strength_ok(const char *s) {
    size_t L = strlen(s);
    if (L < 8) return 0;
    int lower = 0, upper = 0, digit = 0, special = 0;
    for (size_t i=0;i<L;i++) {
        unsigned char c = (unsigned char)s[i];
        if (islower(c)) lower = 1;
        else if (isupper(c)) upper = 1;
        else if (isdigit(c)) digit = 1;
        else special = 1;
    }
    return lower && upper && digit && special;
}

/* ----------------------------- PHONEBOOK API --------------------------- */

static void pb_init(phonebook *pb) {
    pb->size = 0;
    pb->capacity = 8;
    pb->data = calloc(pb->capacity, sizeof(person));
    if (!pb->data) { perror("calloc"); exit(EXIT_FAILURE); }
}

static void pb_free(phonebook *pb) {
    if (pb->data) free(pb->data);
    pb->data = NULL;
    pb->size = pb->capacity = 0;
}

static void pb_ensure_capacity(phonebook *pb) {
    if (pb->size >= pb->capacity) {
        pb->capacity *= 2;
        person *tmp = realloc(pb->data, pb->capacity * sizeof(person));
        if (!tmp) { perror("realloc"); exit(EXIT_FAILURE); }
        pb->data = tmp;
    }
}

static void pb_add(phonebook *pb, const person *p) {
    pb_ensure_capacity(pb);
    pb->data[pb->size++] = *p;
}

static void pb_delete_at(phonebook *pb, size_t idx) {
    if (idx >= pb->size) return;
    
    /* Move elements down to fill the gap */
    for (size_t i = idx + 1; i < pb->size; i++) {
        pb->data[i - 1] = pb->data[i];
    }
    pb->size--;
    
    /* If we're using less than 25% of capacity and capacity > 16, shrink */
    if (pb->size > 0 && pb->capacity > 16 && pb->size * 4 < pb->capacity) {
        size_t new_capacity = pb->capacity / 2;
        if (new_capacity < 8) new_capacity = 8;
        person *tmp = realloc(pb->data, new_capacity * sizeof(person));
        if (tmp) {
            pb->data = tmp;
            pb->capacity = new_capacity;
        }
    }
    
    /* Clear the now-unused element */
    if (pb->size < pb->capacity) {
        memset(&pb->data[pb->size], 0, sizeof(person));
    }
}

/* ----------------------------- SAVE / LOAD ---------------------------- */

/* Save full phonebook to binary file */
static int save_to_file(phonebook *pb) {
    FILE *fp = fopen(DATA_FILE, "wb");
    if (!fp) { perror("fopen"); return 0; }
    uint64_t cnt = (uint64_t)pb->size;
    if (fwrite(&cnt, sizeof(cnt), 1, fp) != 1) { fclose(fp); return 0; }
    if (pb->size > 0) {
        if (fwrite(pb->data, sizeof(person), pb->size, fp) != pb->size) { fclose(fp); return 0; }
    }
    fclose(fp);
    return 1;
}

/* Load file, return number of entries loaded or -1 on error/no-file */
static int load_from_file(phonebook *pb) {
    FILE *fp = fopen(DATA_FILE, "rb");
    if (!fp) return -1;
    uint64_t cnt = 0;
    if (fread(&cnt, sizeof(cnt), 1, fp) != 1) { fclose(fp); return -1; }
    if (cnt == 0) { pb->size = 0; fclose(fp); return 0; }
    while (pb->capacity < (size_t)cnt) pb_ensure_capacity(pb);
    if (fread(pb->data, sizeof(person), (size_t)cnt, fp) != (size_t)cnt) { fclose(fp); return -1; }
    pb->size = (size_t)cnt;
    fclose(fp);
    return (int)pb->size;
}

/* Display a short animated progress bar (visual polish) */
static void show_progress_bar(const char *label, unsigned int duration_ms) {
    const int width = 30;
    unsigned int step_ms = 50;
    unsigned int steps = duration_ms / step_ms;
    printf("%s ", label);
    fflush(stdout);
    for (unsigned int i=0;i<=steps;i++) {
        float frac = (float)i / (float)steps;
        int filled = (int)(frac * width + 0.5f);
        printf("[");
        for (int j=0;j<width;j++) putchar(j < filled ? '=' : ' ');
        printf("] %3d%%\r", (int)(frac * 100.0f));
        fflush(stdout);
        msleep(step_ms);
    }
    printf("\n");
}

/* ----------------------------- UI / PRINT HELPERS --------------------- */

/* boxed title for main banner / sections */
static void print_boxed_title(const char *title) {
    int width = 64;
    int pad = (width - (int)strlen(title) - 2) / 2; /* -2 for spaces */
    printf(COL_BLUE COL_BOLD "┌"); for (int i=0;i<width-2;i++) printf("─"); printf("┐\n" COL_RESET);
    printf(COL_BLUE COL_BOLD "│" COL_RESET);
    for (int i=0;i<pad;i++) printf(" ");
    printf(COL_CYAN "%s" COL_RESET, title);
    for (int i=0;i<width - 2 - pad - (int)strlen(title); i++) printf(" ");
    printf(COL_BLUE COL_BOLD "│\n" COL_RESET);
    printf(COL_BLUE COL_BOLD "└"); for (int i=0;i<width-2;i++) printf("─"); printf("┘\n" COL_RESET);
}

/* divider line */
static void print_divider(void) {
    printf(COL_FAINT "────────────────────────────────────────────────────────────────\n" COL_RESET);
}

/* center a short line (used for marketing-ish header) */
static void print_centered_line(const char *s) {
    int width = 64;
    int len = (int)strlen(s);
    int left = (width - len) / 2;
    for (int i=0;i<left;i++) printf(" ");
    printf("%s\n", s);
}

/* table header for view all */
static void print_table_header(void) {
    printf(COL_BOLD);
    printf("┌────┬────────────────────┬───────────────┬────────────────────────────┐\n");
    printf("│ ID │ Name               │ Phone         │ Email                      │\n");
    printf("├────┼────────────────────┼───────────────┼────────────────────────────┤\n");
    printf(COL_RESET);
}

/* truncate printing helper */
static void print_trunc(const char *s, int width) {
    int L = (int)strlen(s);
    if (L <= width) printf("%-*s", width, s);
    else {
        char tmp[256];
        snprintf(tmp, sizeof(tmp), "%.*s", width - 3, s);
        printf("%s...", tmp);
    }
}

/* ----------------------------- AUTHENTICATION ------------------------ */

/* Returns index of matching contact or -1 */
static int authenticate(phonebook *pb, int id, const char *password) {
    for (size_t i=0;i<pb->size;i++) {
        if (pb->data[i].detail.id_no == id && strcmp(pb->data[i].password, password) == 0) {
            return (int)i;
        }
    }
    return -1;
}

/* ----------------------------- CORE FEATURES ------------------------- */

/* Add contact interactively */
static void add_contact(phonebook *pb) {
    person p;
    memset(&p, 0, sizeof(p));

    print_boxed_title(" Add New Contact ");
    printf(COL_YELLOW "Tip: you can press Enter after each field. Fields accept spaces.\n" COL_RESET);

    while (1) {
        printf("\nName (2-99 chars): ");
        fgets_trim(p.name, sizeof(p.name));
        if (validate_string_length(p.name, 2, 99)) break;
        printf(COL_RED "Name must be between 2 and 99 characters.\n" COL_RESET);
    }

    /* phone */
    char phone_buf[128];
    while (1) {
        printf("Phone (6-15 digits, optional +): ");
        fgets_trim(phone_buf, sizeof(phone_buf));
        if (validate_phone_str(phone_buf)) {
            char tmp[64]; size_t k=0;
            for (size_t i=0;i<strlen(phone_buf);i++) if (phone_buf[i] != '+') tmp[k++] = phone_buf[i];
            tmp[k] = '\0';
            char *endptr;
            errno = 0;
            p.ph_no = strtoll(tmp, &endptr, 10);
            if (*endptr != '\0' || errno == ERANGE) {
                printf(COL_RED "Invalid phone number format. Please try again.\n" COL_RESET);
                continue;
            }
            break;
        } else {
            printf(COL_RED "Invalid phone format. Please try again.\n" COL_RESET);
        }
    }

    /* password with masking and strength checks */
    while (1) {
        char pw[64], pw2[64];
        get_password_masked(pw, sizeof(pw), "Set password (min8, include upper/lower/digit/special): ");
        get_password_masked(pw2, sizeof(pw2), "Confirm password: ");
        if (strcmp(pw, pw2) != 0) {
            printf(COL_YELLOW "Passwords did not match. Try again.\n" COL_RESET);
            continue;
        }
        if (!password_strength_ok(pw)) {
            printf(COL_YELLOW "Password too weak. Please follow rules.\n" COL_RESET);
            continue;
        }
        strncpy(p.password, pw, sizeof(p.password)-1);
        break;
    }

    /* email */
    while (1) {
        printf("Email: ");
        fgets_trim(p.gmail, sizeof(p.gmail));
        if (validate_email(p.gmail)) break;
        printf(COL_RED "Invalid email. Example: someone@example.com\n" COL_RESET);
    }

    /* age */
    while (1) {
        char tmp[32];
        printf("Age: ");
        fgets_trim(tmp, sizeof(tmp));
        int age = atoi(tmp);
        if (age > 0 && age < 130) { p.detail.age = age; break; }
        printf(COL_RED "Invalid age. Enter a valid number.\n" COL_RESET);
    }

    /* ID */
    while (1) {
        char tmp[32];
        printf("ID number (unique integer): ");
        fgets_trim(tmp, sizeof(tmp));
        
        /* Validate input format */
        if (!validate_numeric_input(tmp, 1, INT_MAX)) {
            printf(COL_RED "Invalid ID. Must be a positive integer.\n" COL_RESET);
            continue;
        }
        
        int id = atoi(tmp);
        
        /* Check uniqueness */
        int is_unique = 1;
        for (size_t i = 0; i < pb->size; i++) {
            if (pb->data[i].detail.id_no == id) {
                printf(COL_RED "ID %d already exists. Please choose a different ID.\n" COL_RESET, id);
                is_unique = 0;
                break;
            }
        }
        
        if (is_unique) {
            p.detail.id_no = id;
            break;
        }
    }

    /* free-text gender and religion */
    printf("Gender (free text): ");
    fgets_trim(p.detail.gender, sizeof(p.detail.gender));
    printf("Religion (free text): ");
    fgets_trim(p.detail.religion, sizeof(p.detail.religion));

    /* address */
    char tmp_home[32];
    printf("Address - Home number (int): ");
    fgets_trim(tmp_home, sizeof(tmp_home));
    p.detail.place.home_no = atoi(tmp_home);
    printf("Neighbourhood: ");
    fgets_trim(p.detail.place.neighbourhood, sizeof(p.detail.place.neighbourhood));
    printf("Road: ");
    fgets_trim(p.detail.place.road, sizeof(p.detail.place.road));
    printf("City: ");
    fgets_trim(p.detail.place.city, sizeof(p.detail.place.city));
    printf("State: ");
    fgets_trim(p.detail.place.state, sizeof(p.detail.place.state));
    printf("Country: ");
    fgets_trim(p.detail.place.country, sizeof(p.detail.place.country));

    /* job */
    printf("Job role name: ");
    fgets_trim(p.job.role_name, sizeof(p.job.role_name));
    while (1) {
        char tmp[32];
        printf("Experience (years integer): ");
        fgets_trim(tmp, sizeof(tmp));
        int exp = atoi(tmp);
        if (exp >= 0 && exp <= 50) { p.job.experience_years = exp; break; }
        printf(COL_RED "Invalid experience. Enter 0..50.\n" COL_RESET);
    }
    printf("Company: ");
    fgets_trim(p.job.company, sizeof(p.job.company));
    while (1) {
        char tmp[64];
        printf("Salary (numeric): ");
        fgets_trim(tmp, sizeof(tmp));
        float sal = (float)atof(tmp);
        if (sal >= 0.0f) { p.job.salary = sal; break; }
        printf(COL_RED "Invalid salary. Try again.\n" COL_RESET);
    }

    pb_add(pb, &p);
    printf(COL_GREEN "✅ Contact \"%s\" added successfully!\n" COL_RESET, p.name);
}

/* View all contacts in a nice table */
static void view_all_contacts(phonebook *pb) {
    print_boxed_title(" Contacts Overview ");
    if (pb->size == 0) {
        printf(COL_YELLOW "No contacts saved yet. Add one from the menu.\n" COL_RESET);
        return;
    }

    print_table_header();
    for (size_t i=0;i<pb->size;i++) {
        printf("│ %2zu │ ", i+1);
        print_trunc(pb->data[i].name, 18); printf(" │ ");
        char phone_buf[32]; snprintf(phone_buf, sizeof(phone_buf), "%lld", pb->data[i].ph_no);
        print_trunc(phone_buf, 13); printf(" │ ");
        print_trunc(pb->data[i].gmail, 26); printf(" │\n");
    }
    printf("└────┴────────────────────┴───────────────┴────────────────────────────┘\n");
}

/* Show details after authentication; returns 1 if success */
static int show_details_with_auth(phonebook *pb, size_t idx) {
    if (idx >= pb->size) return 0;
    char idbuf[32];
    printf("Enter ID to view details: ");
    fgets_trim(idbuf, sizeof(idbuf));
    int id = atoi(idbuf);

    int tries = 3;
    while (tries--) {
        char pw[64];
        get_password_masked(pw, sizeof(pw), "Enter password: ");
        int auth_idx = authenticate(pb, id, pw);
        if (auth_idx == (int)idx) {
            /* print full details */
            person *p = &pb->data[idx];
            print_boxed_title(" Protected Details ");
            printf(COL_CYAN "Name       : " COL_RESET "%s\n", p->name);
            printf(COL_CYAN "Age        : " COL_RESET "%d\n", p->detail.age);
            printf(COL_CYAN "ID         : " COL_RESET "%d\n", p->detail.id_no);
            printf(COL_CYAN "Gender     : " COL_RESET "%s\n", p->detail.gender);
            printf(COL_CYAN "Religion   : " COL_RESET "%s\n", p->detail.religion);
            printf(COL_CYAN "Phone      : " COL_RESET "%lld\n", p->ph_no);
            printf(COL_CYAN "Email      : " COL_RESET "%s\n", p->gmail);
            printf("\n" COL_MAGENTA "Address:\n" COL_RESET);
            printf("  Home no      : %d\n", p->detail.place.home_no);
            printf("  Neighbourhood: %s\n", p->detail.place.neighbourhood);
            printf("  Road         : %s\n", p->detail.place.road);
            printf("  City         : %s\n", p->detail.place.city);
            printf("  State        : %s\n", p->detail.place.state);
            printf("  Country      : %s\n", p->detail.place.country);
            printf("\n" COL_MAGENTA "Job:\n" COL_RESET);
            printf("  Role         : %s\n", p->job.role_name);
            printf("  Experience   : %d years\n", p->job.experience_years);
            printf("  Company      : %s\n", p->job.company);
            printf("  Salary       : %.2f\n", p->job.salary);
            printf("  Est. Tax     : %.2f\n", p->job.salary * 0.3f);
            return 1;
        } else {
            if (tries > 0) printf(COL_YELLOW "Incorrect password — %d attempt(s) left.\n" COL_RESET, tries);
            else printf(COL_RED "Authentication failed. Access denied.\n" COL_RESET);
        }
    }
    return 0;
}

/* Search for contact: exact match then prefix suggestions */
static void search_contact(phonebook *pb) {
    print_boxed_title(" Search ");
    char q[120];
    printf("Enter name (or part): ");
    fgets_trim(q, sizeof(q));
    if (strlen(q) == 0) { printf(COL_YELLOW "Empty query — returning to menu.\n" COL_RESET); return; }

    /* exact match */
    for (size_t i=0;i<pb->size;i++) {
        if (str_case_equal(pb->data[i].name, q)) {
            printf(COL_GREEN "Exact match found: %s\n" COL_RESET, pb->data[i].name);
            printf("Phone: %lld   Email: %s\n", pb->data[i].ph_no, pb->data[i].gmail);
            printf("View protected details? (y/n): ");
            char ans[8]; fgets_trim(ans, sizeof(ans));
            if (tolower((unsigned char)ans[0]) == 'y') show_details_with_auth(pb, i);
            return;
        }
    }

    /* suggestions */
    printf(COL_YELLOW "No exact match. Suggestions (prefix-based):\n" COL_RESET);
    int found = 0;
    for (size_t i=0;i<pb->size;i++) {
        if (str_case_prefix(q, pb->data[i].name)) {
            printf("  [%zu] %s  |  %lld  |  %s\n", i+1, pb->data[i].name, pb->data[i].ph_no, pb->data[i].gmail);
            found = 1;
        }
    }
    if (!found) { printf("  (no similar names found)\n"); return; }

    printf("Open one by index or 0 to cancel: ");
    char idxbuf[32]; fgets_trim(idxbuf, sizeof(idxbuf));
    int idx = atoi(idxbuf);
    if (idx <= 0 || (size_t)idx > pb->size) { printf("Canceled.\n"); return; }
    show_details_with_auth(pb, (size_t)(idx - 1));
}

/* Delete a contact by index (with confirmation) */
static void delete_contact(phonebook *pb) {
    print_boxed_title(" Delete Contact ");
    if (pb->size == 0) { printf(COL_YELLOW "Phonebook is empty.\n" COL_RESET); return; }
    view_all_contacts(pb);
    printf("Enter index to delete (1..%zu) or 0 to cancel: ", pb->size);
    char tmp[32]; fgets_trim(tmp, sizeof(tmp));
    int idx = atoi(tmp);
    if (idx <= 0 || (size_t)idx > pb->size) { printf("Cancelled.\n"); return; }
    idx--;
    printf(COL_RED "Confirm deletion of '%s' (y/n): " COL_RESET, pb->data[idx].name);
    char ans[8]; fgets_trim(ans, sizeof(ans));
    if (tolower((unsigned char)ans[0]) == 'y') {
        pb_delete_at(pb, (size_t)idx);
        printf(COL_GREEN "Deleted successfully.\n" COL_RESET);
    } else {
        printf("Cancelled.\n");
    }
}

/* ----------------------------- HELPERS / DISPLAY ---------------------- */

/* Print the main banner (visual dashboard) */
static void print_main_banner(void) {
    enable_ansi_on_windows();
    print_boxed_title(" PHONEBOOK PRO • UX EDITION ");
    print_centered_line(COL_BOLD "A clean, humane, terminal-first contact manager" COL_RESET);
    print_divider();
    printf("Quick commands: %s[1]%s Add   %s[2]%s Search   %s[3]%s ViewAll   %s[5]%s Save\n\n",
           COL_CYAN, COL_RESET, COL_CYAN, COL_RESET, COL_CYAN, COL_RESET, COL_CYAN, COL_RESET);
}



/* ----------------------------- MAIN MENU ------------------------------ */

static void print_menu(void) {
    printf("\n" COL_CYAN "╔═ Menu ═════════════════════════════════════════════════════════╗\n" COL_RESET);
    printf("  %s1%s) Add New Contact    %s2%s) Search Contact    %s3%s) View All\n",
           COL_YELLOW, COL_RESET, COL_YELLOW, COL_RESET, COL_YELLOW, COL_RESET);
    printf("  %s4%s) Delete Contact     %s5%s) Save to disk      %s6%s) Load from disk\n",
           COL_YELLOW, COL_RESET, COL_YELLOW, COL_RESET, COL_YELLOW, COL_RESET);
    printf("  %s0%s) Exit\n", COL_YELLOW, COL_RESET);
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("Choice: ");
}

/* ----------------------------- MAIN ---------------------------------- */

int main(void) {
    phonebook pb;
    pb_init(&pb);

    print_main_banner();

    /* attempt auto-load (friendly) */
    int res = load_from_file(&pb);
    if (res > 0) {
        printf(COL_GREEN "Auto-loaded %d entries from %s\n" COL_RESET, res, DATA_FILE);
    } else if (res == 0) {
        printf(COL_YELLOW "Save file exists but empty.\n" COL_RESET);
    } else {
        printf(COL_YELLOW "No save file found — start by adding contacts.\n" COL_RESET);
    }

    while (1) {
        print_menu();
        char choice_buf[32];
        fgets_trim(choice_buf, sizeof(choice_buf));
        int choice = atoi(choice_buf);

        switch (choice) {
            case 1:
                add_contact(&pb);
                break;
            case 2:
                search_contact(&pb);
                break;
            case 3:
                view_all_contacts(&pb);
                break;
            case 4:
                delete_contact(&pb);
                break;
            case 5:
                show_progress_bar(COL_CYAN "Saving to disk..." COL_RESET, 800);
                if (save_to_file(&pb)) printf(COL_GREEN "✅ Saved to %s (%zu entries)\n" COL_RESET, DATA_FILE, pb.size);
                else printf(COL_RED "✗ Failed to save to %s\n" COL_RESET, DATA_FILE);
                break;
            case 6:
                printf(COL_YELLOW "Warning: loading will replace current in-memory entries. Continue? (y/n): " COL_RESET);
                {
                    char c[8]; fgets_trim(c, sizeof(c));
                    if (tolower((unsigned char)c[0]) == 'y') {
                        show_progress_bar(COL_CYAN "Loading from disk..." COL_RESET, 800);
                        int loaded = load_from_file(&pb);
                        if (loaded >= 0) printf(COL_GREEN "✅ Loaded %d entries\n" COL_RESET, loaded);
                        else printf(COL_RED "✗ Failed to load (file missing or corrupted)\n" COL_RESET);
                    } else printf("Load canceled.\n");
                }
                break;
            case 0:
                printf(COL_YELLOW "Would you like to save before exit? (y/n): " COL_RESET);
                {
                    char c[8]; fgets_trim(c, sizeof(c));
                    if (tolower((unsigned char)c[0]) == 'y') {
                        show_progress_bar(COL_CYAN "Saving before exit..." COL_RESET, 800);
                        if (save_to_file(&pb)) printf(COL_GREEN "✅ Saved to %s\n" COL_RESET, DATA_FILE);
                        else printf(COL_RED "✗ Failed to save\n" COL_RESET);
                    }
                }
                print_boxed_title(" Goodbye — Phonebook Pro ");
                pb_free(&pb);
                return 0;
            default:
                printf(COL_RED "Invalid choice — please enter a number from the menu.\n" COL_RESET);
        }
    }

    /* unreachable */
    pb_free(&pb);
    return 0;
}
