#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define LAUNCH_ID 176

void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void initialize_bubbles();
void draw_bubble(int xc, int yc, int r, int colour, bool pop);
void erase_bubble(int bubbleIndex);
void pop_bubble(int bubbleIndex);
void update_game_board();
void reset_launch_bubble();
void reset_visited();
void check_pop(int bubble_ID);
void draw_bubble_boundary(int xc, int yc, int x, int y, int colour);
void draw_line(int x0, int y0, int x1, int y1, short int line_color);
void swap(int* number1, int* number2);
void wait_loop();
bool check_game_over();

volatile int pixel_buffer_start; // global variable
volatile int pixel_buffer_back;
volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
volatile int* pixel_status_ptr = (int *)0xFF20302C;
volatile int *hex3_0_ptr = (int *)0xFF200020;

typedef struct Bubble { 
   int xc;
   int yc;
   int radius;
   int colour;
   bool visible;
   int up;
   int down;
   int right;
   int left;
} Bubble;

Bubble bubbles[177];
bool visited[176];;
int bubble_colour[3] = {0xC8A4FF, 0xF81F, 0xFF876D};
int entered_recursive;


int main(void)
{
	pixel_buffer_start = *pixel_ctrl_ptr;
	pixel_buffer_back = *pixel_ctrl_ptr;
		
	clear_screen();
	
	initialize_bubbles();
	
	bool game_over = false;
	
	while(!game_over){
		
		reset_launch_bubble();
		
		reset_visited();
		
		draw_bubble(bubbles[LAUNCH_ID].xc, bubbles[LAUNCH_ID].yc, bubbles[LAUNCH_ID].radius, bubbles[LAUNCH_ID].colour, false);
		
		int y = 230;
		bool keep_going = true;
		
		while(keep_going){
			for(int i = 0; i < 176; i++)
				if(bubbles[LAUNCH_ID].xc == bubbles[i].xc 
				&& bubbles[i].yc >= (bubbles[LAUNCH_ID].yc - 25)
				&& bubbles[i].yc <= (bubbles[LAUNCH_ID].yc + 25)
				&& bubbles[i].colour != 0x0)
					keep_going = false;

			wait_loop();
			erase_bubble(LAUNCH_ID);
			bubbles[LAUNCH_ID].yc -= 3;
			y -= 3;
			draw_bubble(169, y, 10, bubbles[LAUNCH_ID].colour, false);

		
		}
		update_game_board();
		game_over = check_game_over();
		//reset_launch_bubble();
		
	}
	
	return 0;
}

void initialize_bubbles(){
	
	int xCount = 0;
	int yCount = 0;
	
	for(int i = 0; i < LAUNCH_ID; i++){

		
		if( i != 0 && i % 16 == 0){
			xCount += 16;
			yCount += 1;
		}
		
		bubbles[i].xc = 20 * (i - xCount) + 9;
		bubbles[i].yc = 10 + (20 * yCount);
		bubbles[i].colour = bubble_colour[rand() % 3];
		bubbles[i].radius = 10;
		bubbles[i].visible = true;
		
		if(i < 16)
			bubbles[i].up = -1;
		else
			bubbles[i].up = i - 16;
			
		if(i > 175)
			bubbles[i].down = -1;
		else
			bubbles[i].down = i + 16;
		
		if(i != 0 && i % 15 == 0)
			bubbles[i].right = -1;
		else
			bubbles[i].right = i + 1;
			
		if(i == 0 || i % 16 == 0)
			bubbles[i].left = -1;
		else
			bubbles[i].left = i - 1;
		
		if(i > 47){
			bubbles[i].colour = 0x0;
			bubbles[i].visible = false;
		}
		
		draw_bubble(bubbles[i].xc, bubbles[i].yc, bubbles[i].radius, bubbles[i].colour, false);
	
	}
	
}

void plot_pixel(int x, int y, short int line_color){
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void clear_screen(){
	
	for(int i = 0; i < 320; i++)
		for(int j = 0; j < 240; j++ )
			plot_pixel(i, j, 0x0);
}

void draw_bubble(int xc, int yc, int r, int colour, bool pop){ 
    int x = 0, y = r; 
    int d = 3 - 2 * r; 
	if(pop)
		colour = 0x0;
    draw_bubble_boundary(xc, yc, x, y, colour); 
    while (y >= x) { 
        x++; 
		
        if (d > 0) { 
            y--;  
            d = d + 4 * (x - y) + 10; 
        } 
        else
            d = d + 4 * x + 6; 
        draw_bubble_boundary(xc, yc, x, y, colour);
		
		if(pop)
			wait_loop();

    }
}
	
void draw_bubble_boundary(int xc, int yc, int x, int y, int colour){ 
	draw_line(xc - x, yc + y, xc + x, yc + y, colour);
	draw_line(xc - x, yc - y, xc + x, yc - y, colour);
	draw_line(xc - y, yc + x, xc + y, yc + x, colour);
	draw_line(xc - y, yc - x, xc + y, yc - x, colour);
}

void draw_line(int x0, int y0, int x1, int y1, short int line_color){
	
	int x, y, deltax, deltay, error, y_step;
	bool isSteep = abs(y1 - y0) > abs(x1 - x0);
	
	//if steep, swap x0,y0 and x1,y1
	if(isSteep){
		swap(&x0,&y0);
        swap(&x1,&y1);
	}
	
	if(x0 > x1){
		swap(&x0, &x1);
		swap(&y0, &y1);
	}
	
	deltax = x1 - x0;
	deltay = abs(y1 - y0);
	error = -(deltax / 2);
	y = y0;
	if(y0 < y1)
		y_step = 1;
	else
		y_step = -1;
	for(x = x0; x < x1; x++){
		if(isSteep)
			plot_pixel(y, x, line_color);			
		else
			plot_pixel(x, y, line_color);
		error += deltay;
		if(error >= 0){
			y += y_step;
			error -= deltax;
		}
	}
}


void erase_bubble(int bubbleIndex){
	bubbles[bubbleIndex].visible = false;
	if(bubbleIndex != LAUNCH_ID)
		bubbles[bubbleIndex].colour = 0x0;
	draw_bubble(bubbles[bubbleIndex].xc, bubbles[bubbleIndex].yc, bubbles[bubbleIndex].radius, 0x0, false);
}

void pop_bubble(int bubbleIndex){
	bubbles[bubbleIndex].visible = false;
	bubbles[bubbleIndex].colour = 0x0;
	draw_bubble(bubbles[bubbleIndex].xc, bubbles[bubbleIndex].yc, bubbles[bubbleIndex].radius, 0x0, true);
}

void reset_launch_bubble(){
	
	bubbles[LAUNCH_ID].xc = 169;
	bubbles[LAUNCH_ID].yc = 230;
	bubbles[LAUNCH_ID].radius = 10;
	bubbles[LAUNCH_ID].colour = bubble_colour[rand() % 3];
	bubbles[LAUNCH_ID].visible = true;
}

void reset_visited(){
	
	for(int i = 0; i < 176; i ++)
		visited[i] = false;
		
	entered_recursive = 0;
}

void update_game_board(){
	for(int i = 0; i < 176; i++){
		if(bubbles[LAUNCH_ID].xc == bubbles[i].xc 
		&& bubbles[LAUNCH_ID].yc >= bubbles[i].yc - 2
		&& bubbles[LAUNCH_ID].yc <= bubbles[i].yc + 2){
			bubbles[i].colour = bubbles[LAUNCH_ID].colour;
			erase_bubble(LAUNCH_ID);
			draw_bubble(bubbles[i].xc, bubbles[i].yc, 10, bubbles[i].colour, false);
			check_pop(i);
		}
	}
}


bool check_game_over(){
	
	for(int i = 169; i < 176; i++)
		if(bubbles[i].colour != 0x0)
			return true;
			
	else
		return false;
}

void check_pop(int bubble_ID){
	
	entered_recursive++;
	
	visited[bubble_ID] = true;
	bool entered_at_least_once = false;
	
	if(bubbles[bubble_ID].colour == bubbles[bubbles[bubble_ID].up].colour){
		entered_at_least_once = true;
		check_pop(bubbles[bubble_ID].up);
	}
	if(bubbles[bubble_ID].colour == bubbles[bubbles[bubble_ID].right].colour){
		entered_at_least_once = true;
		check_pop(bubbles[bubble_ID].right);
	}
	if(bubbles[bubble_ID].colour == bubbles[bubbles[bubble_ID].left].colour
	&& visited[bubbles[bubble_ID].left] == false){
		entered_at_least_once = true;
		check_pop(bubbles[bubble_ID].left);
	}

	if(entered_recursive > 2)	
		erase_bubble(bubble_ID);
	
}

void wait_loop(){
	*pixel_ctrl_ptr = 1;
	do{
	}while(*pixel_status_ptr & 0x01);
}

void swap(int* number1, int* number2){
  int temp = *number1;
  *number1 = *number2;
  *number2 = temp;
}

void display_hex(){
	//char zero = 0b00111111, one = 0b00000110, two = 0b01011011, three = 0b01001111, four = 0b01100110;
	//char five = 0b01101101, six = 0b01111101, seven = 0b00000111, eight = 0b01111111, nine = 0b01100111;
}