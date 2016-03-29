/*
 * compile: gcc maze.c -o run `pkg-config --cflags --libs cairo`
*/

#include <stdio.h>
#include <stdlib.h>
#include <cairo.h>

#define SIZE 50    /* maze is 50x50 cells */

/* simulates tile locations and values represent 'visited' status */
int ** maze;   

void draw_grid(cairo_t *);
void generate_maze(cairo_t *);
void remove_wall(cairo_t *, int, int, int, int);
void make_path(cairo_t *, int, int, int, int);
int check_available(int, int);



int main()
{
    /* Intializes random number generator */
    time_t t;
    srand((unsigned) time(&t));
    
    /* make the drawing canvas 1000x1000 pixels using rgb colors */
    cairo_surface_t * surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1000, 1000);
    cairo_t * cr = cairo_create (surface);    /* our Cairo context is associated with the surface */
  
    draw_grid(cr);       /* initialize the grid */
    generate_maze(cr);   /* generate pathways */
  
    /* destroy Cairo surface and context, and save the resulting maze */
    cairo_destroy (cr);
    cairo_surface_write_to_png (surface, "maze.png");
    cairo_surface_destroy (surface);

    return 0;
}



/* Setup the initial grid with every cell walled in. */
void draw_grid(cairo_t *cr)
{
    /* create a background */
    cairo_set_source_rgb(cr, 0, 0, 0);         /* black */
    cairo_rectangle(cr, 0, 0, 1000, 1000);     /* set the background to cover the entire canvas */
    cairo_stroke_preserve(cr);
    cairo_fill(cr);
  
    cairo_set_source_rgb(cr, 0.05, 0.38, 0.66);         /* change color to blue */
    cairo_set_line_width(cr, 3);                        /* line width is 3px */
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);       /* set lines with rounded endpoints */
  
    /* create a 50x50 cell grid  (each cell is 18x18 px)
      and the padding around the maze is 50px */
    int i;
    for(i = 0; i < 918; i+=18)
    {
        cairo_move_to(cr, 50, 50+i);    /* initialize the starting point */
        cairo_line_to(cr, 950, 50+i);   /* draw horizontal line */
        cairo_stroke(cr);
        cairo_move_to(cr, 50+i, 50);    /* initialize the starting point */
        cairo_line_to(cr, 50+i, 950);   /* draw vertical line */
        cairo_stroke(cr);
    }
}



/* Generates random pathways through the grid using the 
   maze of numbers as a guide. Allocates memory for the maze
   and initializes every tile location to 0/unvisited. */
void generate_maze(cairo_t * cr)
{
    int i;  /* y-coordinate */
    int j;  /* x-coordinate */
    
    maze = (int**)malloc(sizeof(int*)*SIZE);
    
    for(i = 0; i < SIZE; ++i)
        maze[i] = (int*)malloc(sizeof(int)*SIZE);
    
    for(i = 0; i < SIZE; ++i)
    {
        for(j = 0; j < SIZE; ++j)    /* mark each tile as unvisited */
            maze[i][j] = 0;
    }
    
    make_path(cr, 0, 0, 0, 0);   /* recursively generate paths */

    /* deallocate maze */
    for(i = 0; i < SIZE; ++i)
        free(maze[i]);
    free(maze);        
}



/* Recursive function that locates tiles that haven't yet been
   traversed, and if it finds one, it removes the wall between the previous
   and the new tile and the previous tile location is pushed on the stack. 
   When one of these stack frames is returned to, the search for unvisited 
   tiles repeats. */
void make_path(cairo_t * cr, int i, int j, int previ, int prevj)
{
    int next;
    maze[i][j] = 1;   /* mark current tile as visited */
    
    do
    {
        next = check_available(i, j);  /* get the next tile to traverse */
        switch(next)
        {
            case 1: remove_wall(cr, i-1, j, i, j);   /* go up */
                make_path(cr, i-1, j, i, j);
                break;
            case 2: remove_wall(cr, i, j+1, i, j);   /* go right */
                make_path(cr, i, j+1, i, j);
                break;
            case 3: remove_wall(cr, i+1, j, i, j);   /* go down */
                make_path(cr, i+1, j, i, j);
                break;
            case 4: remove_wall(cr, i, j-1, i, j);   /* go left */
                make_path(cr, i, j-1, i, j);
                break;
        }
    }while(next != 0);   /* loop until all surrounding tiles have been visited */
    
    return;
}



/* Checks for unvisited surrounding tiles and return the next tile to 
   traverse. There is a maximum of 3 surrounding tiles to traverse and
   a maximum of 4 directions. */
int check_available(int i, int j)
{
    int next;   /* the next tile to traverse */
    int available[4] = {1, 1, 1, 1};   /* initialize all to visited: up, right, down, left */
    
    /* the if statements prevent us from going out of bounds */
    if(i > 0)
        available[0] = maze[i-1][j];   /* copy 'up' tile's visted status */
    if(j < SIZE-1)
        available[1] = maze[i][j+1];   /* copy 'right' tile's visted status */
    if(i < SIZE-1)
        available[2] = maze[i+1][j];   /* copy 'down' tile's visted status */
    if(j > 0)
        available[3] = maze[i][j-1];   /* copy 'left' tile's visted status */

    /* return if there are no tiles available to traverse */
    if(available[0] == 1 && available[1] == 1 && available[2] == 1 && available[3] == 1)
        return 0;
    do
    {
        next = rand() % 4 + 1;        /* number between 1 and 4 */
    }while(available[next-1] != 0);
    
    return next;
}



/* Removes the grid wall between a given a previous and current tile 
   locations. Very pixel specific and each cell must be 18x18 pixels. */
void remove_wall(cairo_t * cr, int i, int j, int previ, int prevj)
{
    cairo_set_source_rgb(cr, 0, 0, 0);   /* black */
    cairo_set_line_width(cr, 4);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT); 
    
    /* coordinates for the center of the previous tile */
    int px = 50 + ((prevj+1)*18) - 9;
    int py = 50 + ((previ+1)*18) - 9;
    
    /* coordinates for the center of the new tile */
    int nx = 50 + ((j+1)*18) - 9;
    int ny = 50 + ((i+1)*18) - 9;
    
    /* the tiles are vertical to each other */
    if(px == nx)
    {
        if(py < ny)  /* previous tile is on top of new tile */
        {
            cairo_move_to(cr, px-7.5, py+9);
            cairo_line_to(cr, px+7.5, py+9);
            cairo_stroke(cr);
        }
        else
        {
            cairo_move_to(cr, nx-7.5, ny+9);
            cairo_line_to(cr, nx+7.5, ny+9);
            cairo_stroke(cr);
        }
    }
    else   /* the tiles are horizontal to each other */
    {
        if(px > nx)  /* previous tile is to the right of the new tile */
        {
            cairo_move_to(cr, px-9, py+7.5);
            cairo_line_to(cr, px-9, py-7.5);
            cairo_stroke(cr);
        }
        else
        {
            cairo_move_to(cr, nx-9, ny+7.5);
            cairo_line_to(cr, nx-9, ny-7.5);
            cairo_stroke(cr);
        }
    }
    
    return;
}



