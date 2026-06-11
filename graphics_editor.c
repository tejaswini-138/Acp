#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

#define ROWS 20
#define COLS 50
#define MAX_SHAPES 100

// Define shape types
typedef enum {
    SHAPE_LINE = 1,
    SHAPE_RECT,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;

// Define structures for shape parameters
typedef struct {
    int x1, y1;
    int x2, y2;
} LineParams;

typedef struct {
    int x, y; // Top-left corner
    int w, h; // Width and height
} RectParams;

typedef struct {
    int cx, cy; // Center
    int r;      // Radius
} CircleParams;

typedef struct {
    int x1, y1;
    int x2, y2;
    int x3, y3;
} TriParams;

// Unified Shape structure
typedef struct {
    int id;
    ShapeType type;
    union {
        LineParams line;
        RectParams rect;
        CircleParams circle;
        TriParams tri;
    } params;
} Shape;

// Global shape list
Shape shapes[MAX_SHAPES];
int shape_count = 0;
int next_shape_id = 1;

// Function declarations
void enable_ansi_escapes();
void init_canvas(char canvas[ROWS][COLS]);
void display_canvas(char canvas[ROWS][COLS]);
void draw_line_on_canvas(char canvas[ROWS][COLS], int x1, int y1, int x2, int y2);
void draw_rect_on_canvas(char canvas[ROWS][COLS], int x, int y, int w, int h);
void draw_circle_on_canvas(char canvas[ROWS][COLS], int cx, int cy, int r);
void draw_triangle_on_canvas(char canvas[ROWS][COLS], int x1, int y1, int x2, int y2, int x3, int y3);
void render_all_shapes(char canvas[ROWS][COLS]);
void list_shapes();
void add_shape();
void modify_shape(int id);
void delete_shape(int id);
void save_to_file(char canvas[ROWS][COLS]);
int find_shape_index_by_id(int id);
int read_int_in_range(const char *prompt, int min_val, int max_val);
void read_string(char *buffer, int max_len);

// Main execution loop
int main() {
    #ifdef _WIN32
    enable_ansi_escapes();
    #endif

    char canvas[ROWS][COLS];

    while (1) {
        // Clear terminal screen and move cursor to home position
        printf("\033[H\033[2J");
        printf("\033[1;36m====================== 2D GRAPHICS EDITOR ======================\033[0m\n\n");
        
        render_all_shapes(canvas);
        display_canvas(canvas);
        printf("\n");
        list_shapes();
        printf("\n");

        printf("\033[1;36mMain Menu:\033[0m\n");
        printf("1. Add Shape\n");
        printf("2. Modify Shape\n");
        printf("3. Delete Shape\n");
        printf("4. Clear Canvas\n");
        printf("5. Save Drawing to File\n");
        printf("6. Exit\n");

        int choice = read_int_in_range("\nEnter action (1-6): ", 1, 6);

        if (choice == 6) {
            printf("\nExiting 2D Graphics Editor. Goodbye!\n");
            break;
        }

        switch (choice) {
            case 1:
                add_shape();
                printf("\nPress Enter to continue...");
                getchar();
                break;
            case 2: {
                if (shape_count == 0) {
                    printf("\033[1;31mNo shapes to modify.\033[0m\n");
                } else {
                    int id = read_int_in_range("Enter Shape ID to modify: ", 1, 10000);
                    modify_shape(id);
                }
                printf("\nPress Enter to continue...");
                getchar();
                break;
            }
            case 3: {
                if (shape_count == 0) {
                    printf("\033[1;31mNo shapes to delete.\033[0m\n");
                } else {
                    int id = read_int_in_range("Enter Shape ID to delete: ", 1, 10000);
                    delete_shape(id);
                }
                printf("\nPress Enter to continue...");
                getchar();
                break;
            }
            case 4:
                shape_count = 0;
                printf("\033[1;32mAll shapes cleared from canvas.\033[0m\n");
                printf("\nPress Enter to continue...");
                getchar();
                break;
            case 5:
                save_to_file(canvas);
                printf("\nPress Enter to continue...");
                getchar();
                break;
        }
    }
    return 0;
}

// Enable ANSI virtual terminal processing on Windows consoles
void enable_ansi_escapes() {
    #ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    #endif
}

// Safe string reading from stdin
void read_string(char *buffer, int max_len) {
    if (fgets(buffer, max_len, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
    }
}

// Safe integer parsing with prompt and range validation
int read_int_in_range(const char *prompt, int min_val, int max_val) {
    char buffer[100];
    int val;
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            continue;
        }
        char *endptr;
        val = (int)strtol(buffer, &endptr, 10);
        // Ensure conversion took place and there are no stray non-whitespace characters
        if (endptr == buffer || (*endptr != '\0' && *endptr != '\n' && *endptr != '\r')) {
            printf("\033[1;31mInvalid format. Please enter an integer.\033[0m\n");
            continue;
        }
        if (val < min_val || val > max_val) {
            printf("\033[1;31mOut of bounds. Must be between %d and %d.\033[0m\n", min_val, max_val);
            continue;
        }
        return val;
    }
}

// Initialize canvas with underscores
void init_canvas(char canvas[ROWS][COLS]) {
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            canvas[y][x] = '_';
        }
    }
}

// Render the 2D array canvas with coordinate guidelines
void display_canvas(char canvas[ROWS][COLS]) {
    // Print column tens markers
    printf("   ");
    for (int x = 0; x < COLS; x++) {
        if (x % 10 == 0) {
            printf("%d", x / 10);
        } else {
            printf(" ");
        }
    }
    printf("\n   ");
    // Print column units markers
    for (int x = 0; x < COLS; x++) {
        printf("%d", x % 10);
    }
    printf("\n");

    // Top border line
    printf("  +");
    for (int x = 0; x < COLS; x++) {
        printf("-");
    }
    printf("+\n");

    // Print rows with row index prefix
    for (int y = 0; y < ROWS; y++) {
        printf("%02d|", y);
        for (int x = 0; x < COLS; x++) {
            char c = canvas[y][x];
            if (c == '*') {
                printf("\033[1;33m*\033[0m"); // Bold Yellow for graphics
            } else {
                printf("\033[90m_\033[0m");  // Dark Gray for canvas grid background
            }
        }
        printf("|\n");
    }

    // Bottom border line
    printf("  +");
    for (int x = 0; x < COLS; x++) {
        printf("-");
    }
    printf("+\n");
}

// Bresenham's Line Algorithm implementation
void draw_line_on_canvas(char canvas[ROWS][COLS], int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1);
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    int e2;

    while (1) {
        if (x1 >= 0 && x1 < COLS && y1 >= 0 && y1 < ROWS) {
            canvas[y1][x1] = '*';
        }
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x1 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// Rectangle Drawing Function (draw borders)
void draw_rect_on_canvas(char canvas[ROWS][COLS], int x, int y, int w, int h) {
    // Top & bottom lines
    for (int i = 0; i < w; i++) {
        int px = x + i;
        if (px >= 0 && px < COLS) {
            if (y >= 0 && y < ROWS) canvas[y][px] = '*';
            if (y + h - 1 >= 0 && y + h - 1 < ROWS) canvas[y + h - 1][px] = '*';
        }
    }
    // Left & right lines
    for (int i = 0; i < h; i++) {
        int py = y + i;
        if (py >= 0 && py < ROWS) {
            if (x >= 0 && x < COLS) canvas[py][x] = '*';
            if (x + w - 1 >= 0 && x + w - 1 < COLS) canvas[py][x + w - 1] = '*';
        }
    }
}

// Midpoint Circle Algorithm Helper
static void plot_circle_points(char canvas[ROWS][COLS], int cx, int cy, int x, int y) {
    int points[8][2] = {
        {cx + x, cy + y}, {cx - x, cy + y}, {cx + x, cy - y}, {cx - x, cy - y},
        {cx + y, cy + x}, {cx - y, cy + x}, {cx + y, cy - x}, {cx - y, cy - x}
    };
    for (int i = 0; i < 8; i++) {
        int px = points[i][0];
        int py = points[i][1];
        if (px >= 0 && px < COLS && py >= 0 && py < ROWS) {
            canvas[py][px] = '*';
        }
    }
}

// Midpoint Circle Algorithm implementation
void draw_circle_on_canvas(char canvas[ROWS][COLS], int cx, int cy, int r) {
    if (r < 0) return;
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    plot_circle_points(canvas, cx, cy, x, y);
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        plot_circle_points(canvas, cx, cy, x, y);
    }
}

// Triangle Drawing Function (renders three lines)
void draw_triangle_on_canvas(char canvas[ROWS][COLS], int x1, int y1, int x2, int y2, int x3, int y3) {
    draw_line_on_canvas(canvas, x1, y1, x2, y2);
    draw_line_on_canvas(canvas, x2, y2, x3, y3);
    draw_line_on_canvas(canvas, x3, y3, x1, y1);
}

// Re-renders the canvas using the current shape configurations
void render_all_shapes(char canvas[ROWS][COLS]) {
    init_canvas(canvas);
    for (int i = 0; i < shape_count; i++) {
        Shape s = shapes[i];
        switch (s.type) {
            case SHAPE_LINE:
                draw_line_on_canvas(canvas, s.params.line.x1, s.params.line.y1, s.params.line.x2, s.params.line.y2);
                break;
            case SHAPE_RECT:
                draw_rect_on_canvas(canvas, s.params.rect.x, s.params.rect.y, s.params.rect.w, s.params.rect.h);
                break;
            case SHAPE_CIRCLE:
                draw_circle_on_canvas(canvas, s.params.circle.cx, s.params.circle.cy, s.params.circle.r);
                break;
            case SHAPE_TRIANGLE:
                draw_triangle_on_canvas(canvas, s.params.tri.x1, s.params.tri.y1, s.params.tri.x2, s.params.tri.y2, s.params.tri.x3, s.params.tri.y3);
                break;
        }
    }
}

// Lists active shapes with descriptions
void list_shapes() {
    if (shape_count == 0) {
        printf("\033[1;33mNo active shapes on the canvas.\033[0m\n");
        return;
    }
    printf("\033[1;36mActive Shapes List:\033[0m\n");
    printf("----------------------------------------------------------------------\n");
    for (int i = 0; i < shape_count; i++) {
        Shape s = shapes[i];
        switch (s.type) {
            case SHAPE_LINE:
                printf("[ID %d] Line: (%d, %d) to (%d, %d)\n", s.id, s.params.line.x1, s.params.line.y1, s.params.line.x2, s.params.line.y2);
                break;
            case SHAPE_RECT:
                printf("[ID %d] Rectangle: Top-Left (%d, %d), Size %dx%d\n", s.id, s.params.rect.x, s.params.rect.y, s.params.rect.w, s.params.rect.h);
                break;
            case SHAPE_CIRCLE:
                printf("[ID %d] Circle: Center (%d, %d), Radius %d\n", s.id, s.params.circle.cx, s.params.circle.cy, s.params.circle.r);
                break;
            case SHAPE_TRIANGLE:
                printf("[ID %d] Triangle: Vertices (%d, %d), (%d, %d), (%d, %d)\n", s.id,
                       s.params.tri.x1, s.params.tri.y1,
                       s.params.tri.x2, s.params.tri.y2,
                       s.params.tri.x3, s.params.tri.y3);
                break;
        }
    }
    printf("----------------------------------------------------------------------\n");
}

// Menu handler for adding a new shape
void add_shape() {
    if (shape_count >= MAX_SHAPES) {
        printf("\033[1;31mError: Canvas shape memory is full (%d shapes max).\033[0m\n", MAX_SHAPES);
        return;
    }

    printf("\n\033[1;36mSelect Shape to Add:\033[0m\n");
    printf("1. Line\n");
    printf("2. Rectangle\n");
    printf("3. Circle\n");
    printf("4. Triangle\n");
    printf("5. [Cancel]\n");
    
    int choice = read_int_in_range("Enter selection (1-5): ", 1, 5);
    if (choice == 5) {
        printf("Cancelled.\n");
        return;
    }

    Shape s;
    s.id = next_shape_id++;
    s.type = (ShapeType)choice;

    switch (s.type) {
        case SHAPE_LINE:
            printf("\033[1;36mAdding Line:\033[0m\n");
            s.params.line.x1 = read_int_in_range("  Enter Start X1 (0-49): ", 0, COLS - 1);
            s.params.line.y1 = read_int_in_range("  Enter Start Y1 (0-19): ", 0, ROWS - 1);
            s.params.line.x2 = read_int_in_range("  Enter End X2 (0-49): ", 0, COLS - 1);
            s.params.line.y2 = read_int_in_range("  Enter End Y2 (0-19): ", 0, ROWS - 1);
            break;
        case SHAPE_RECT:
            printf("\033[1;36mAdding Rectangle:\033[0m\n");
            s.params.rect.x = read_int_in_range("  Enter Top-Left Corner X (0-49): ", 0, COLS - 1);
            s.params.rect.y = read_int_in_range("  Enter Top-Left Corner Y (0-19): ", 0, ROWS - 1);
            s.params.rect.w = read_int_in_range("  Enter Width (1-50): ", 1, COLS);
            s.params.rect.h = read_int_in_range("  Enter Height (1-20): ", 1, ROWS);
            break;
        case SHAPE_CIRCLE:
            printf("\033[1;36mAdding Circle:\033[0m\n");
            s.params.circle.cx = read_int_in_range("  Enter Center X (0-49): ", 0, COLS - 1);
            s.params.circle.cy = read_int_in_range("  Enter Center Y (0-19): ", 0, ROWS - 1);
            s.params.circle.r = read_int_in_range("  Enter Radius (0-25): ", 0, 25);
            break;
        case SHAPE_TRIANGLE:
            printf("\033[1;36mAdding Triangle:\033[0m\n");
            s.params.tri.x1 = read_int_in_range("  Enter Vertex 1 X (0-49): ", 0, COLS - 1);
            s.params.tri.y1 = read_int_in_range("  Enter Vertex 1 Y (0-19): ", 0, ROWS - 1);
            s.params.tri.x2 = read_int_in_range("  Enter Vertex 2 X (0-49): ", 0, COLS - 1);
            s.params.tri.y2 = read_int_in_range("  Enter Vertex 2 Y (0-19): ", 0, ROWS - 1);
            s.params.tri.x3 = read_int_in_range("  Enter Vertex 3 X (0-49): ", 0, COLS - 1);
            s.params.tri.y3 = read_int_in_range("  Enter Vertex 3 Y (0-19): ", 0, ROWS - 1);
            break;
    }

    shapes[shape_count++] = s;
    printf("\033[1;32mShape %d added successfully!\033[0m\n", s.id);
}

// Find a shape's array index by ID
int find_shape_index_by_id(int id) {
    for (int i = 0; i < shape_count; i++) {
        if (shapes[i].id == id) {
            return i;
        }
    }
    return -1;
}

// Menu handler for modifying an existing shape
void modify_shape(int id) {
    int idx = find_shape_index_by_id(id);
    if (idx == -1) {
        printf("\033[1;31mError: Shape with ID %d not found.\033[0m\n", id);
        return;
    }
    
    Shape *s = &shapes[idx];
    printf("\n\033[1;36mModifying Shape ID %d (%s):\033[0m\n", s->id, 
        s->type == SHAPE_LINE ? "Line" : 
        s->type == SHAPE_RECT ? "Rectangle" : 
        s->type == SHAPE_CIRCLE ? "Circle" : "Triangle");
        
    switch (s->type) {
        case SHAPE_LINE:
            printf("Current coordinates: (%d, %d) to (%d, %d)\n", s->params.line.x1, s->params.line.y1, s->params.line.x2, s->params.line.y2);
            s->params.line.x1 = read_int_in_range("  Enter new Start X1 (0-49): ", 0, COLS - 1);
            s->params.line.y1 = read_int_in_range("  Enter new Start Y1 (0-19): ", 0, ROWS - 1);
            s->params.line.x2 = read_int_in_range("  Enter new End X2 (0-49): ", 0, COLS - 1);
            s->params.line.y2 = read_int_in_range("  Enter new End Y2 (0-19): ", 0, ROWS - 1);
            break;
        case SHAPE_RECT:
            printf("Current dimensions: Top-Left (%d, %d), Size %dx%d\n", s->params.rect.x, s->params.rect.y, s->params.rect.w, s->params.rect.h);
            s->params.rect.x = read_int_in_range("  Enter new Top-Left X (0-49): ", 0, COLS - 1);
            s->params.rect.y = read_int_in_range("  Enter new Top-Left Y (0-19): ", 0, ROWS - 1);
            s->params.rect.w = read_int_in_range("  Enter new Width (1-50): ", 1, COLS);
            s->params.rect.h = read_int_in_range("  Enter new Height (1-20): ", 1, ROWS);
            break;
        case SHAPE_CIRCLE:
            printf("Current center & radius: Center (%d, %d), Radius %d\n", s->params.circle.cx, s->params.circle.cy, s->params.circle.r);
            s->params.circle.cx = read_int_in_range("  Enter new Center X (0-49): ", 0, COLS - 1);
            s->params.circle.cy = read_int_in_range("  Enter new Center Y (0-19): ", 0, ROWS - 1);
            s->params.circle.r = read_int_in_range("  Enter new Radius (0-25): ", 0, 25);
            break;
        case SHAPE_TRIANGLE:
            printf("Current vertices: (%d, %d), (%d, %d), (%d, %d)\n",
                s->params.tri.x1, s->params.tri.y1,
                s->params.tri.x2, s->params.tri.y2,
                s->params.tri.x3, s->params.tri.y3);
            s->params.tri.x1 = read_int_in_range("  Enter new Vertex 1 X (0-49): ", 0, COLS - 1);
            s->params.tri.y1 = read_int_in_range("  Enter new Vertex 1 Y (0-19): ", 0, ROWS - 1);
            s->params.tri.x2 = read_int_in_range("  Enter new Vertex 2 X (0-49): ", 0, COLS - 1);
            s->params.tri.y2 = read_int_in_range("  Enter new Vertex 2 Y (0-19): ", 0, ROWS - 1);
            s->params.tri.x3 = read_int_in_range("  Enter new Vertex 3 X (0-49): ", 0, COLS - 1);
            s->params.tri.y3 = read_int_in_range("  Enter new Vertex 3 Y (0-19): ", 0, ROWS - 1);
            break;
    }
    printf("\033[1;32mShape %d modified successfully.\033[0m\n", s->id);
}

// Menu handler for deleting a shape
void delete_shape(int id) {
    int idx = find_shape_index_by_id(id);
    if (idx == -1) {
        printf("\033[1;31mError: Shape with ID %d not found.\033[0m\n", id);
        return;
    }
    
    // Shift all subsequent shapes down by one
    for (int i = idx; i < shape_count - 1; i++) {
        shapes[i] = shapes[i + 1];
    }
    shape_count--;
    printf("\033[1;32mShape %d deleted successfully.\033[0m\n", id);
}

// Save drawing layout to a file
void save_to_file(char canvas[ROWS][COLS]) {
    char filename[100];
    printf("Enter filename to save drawing (default: drawing.txt): ");
    read_string(filename, sizeof(filename));
    
    // Default to drawing.txt if blank
    if (strlen(filename) == 0) {
        strcpy(filename, "drawing.txt");
    }

    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("\033[1;31mError: Could not open file %s for writing.\033[0m\n", filename);
        return;
    }

    // Write top column headers to file
    fprintf(fp, "   ");
    for (int x = 0; x < COLS; x++) {
        if (x % 10 == 0) {
            fprintf(fp, "%d", x / 10);
        } else {
            fprintf(fp, " ");
        }
    }
    fprintf(fp, "\n   ");
    for (int x = 0; x < COLS; x++) {
        fprintf(fp, "%d", x % 10);
    }
    fprintf(fp, "\n  +");
    for (int x = 0; x < COLS; x++) {
        fprintf(fp, "-");
    }
    fprintf(fp, "+\n");

    // Print rows
    for (int y = 0; y < ROWS; y++) {
        fprintf(fp, "%02d|", y);
        for (int x = 0; x < COLS; x++) {
            fputc(canvas[y][x], fp);
        }
        fprintf(fp, "|\n");
    }

    // Print bottom border
    fprintf(fp, "  +");
    for (int x = 0; x < COLS; x++) {
        fprintf(fp, "-");
    }
    fprintf(fp, "+\n");

    fclose(fp);
    printf("\033[1;32mDrawing successfully saved to %s\033[0m\n", filename);
}
