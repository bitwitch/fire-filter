#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define PI 3.14159265358979323846

typedef unsigned char u8;

class Demo : public olc::PixelGameEngine
{
public:
    Demo() { sAppName = "When the fire starts to burn"; }

    int HEIGHT;
    int WIDTH;
    u8 *palette;

    u8 *fire1; 
    u8 *fire2; 
    u8 *tmp;

    olc::Sprite *image;

    int hotspot_start;
    int hotspot_width;

    float current_time;

public:
    // Create a shade of colours in the palette from start to end
    void Shade_Pal( int start, int end, int r1, int g1, int b1, int r2, int g2, int b2 )
    {
        int i;
        float k;
        for (i=0; i<=end-start; i++)
        {
            k = (float)i/(float)(end-start);
            int r_index = (start+i) * 3;
            palette[r_index]   = (u8)(r1+(r2-r1)*k);
            palette[r_index+1] = (u8)(g1+(g2-g1)*k);
            palette[r_index+2] = (u8)(b1+(b2-b1)*k);
        }
    }

    // Adds some hot pixels to a buffer
    void Heat(u8 *dst)
    {
         int i, j;
    // add some random hot spots where the text is
         for (j = 84; j < 200; ++j)
         for (i = 0; i < 320; ++i)
         {
             // since using as a value (greyscale) any color channel will do
             u8 color_val = (u8)image->GetPixel(i, j).r;
             if (color_val > dst[j*WIDTH+i]) 
                 dst[j*WIDTH+i] = rand() & color_val;
         }
         j = (rand() % 1024);
    // add some random hot spots at the bottom of the buffer
    // 
         for (i=0; i<j; i++)
         {
             dst[hotspot_start+(rand()%hotspot_width)] = 255;
         }
    }

    // Smooth a buffer upwards, make sure not to read pixels that are outside of
    // the buffer!
    void Blur_Up(u8 *src, u8 *dst)
    {
         int offs = 0;
         u8 b;
         for (int j=0; j<HEIGHT-2; j++)
         {
             // set first pixel of the line to 0
             dst[offs] = 0; offs++;
         // calculate the filter for all the other pixels
         // 0 0 0 0 0
         // 0 0 0 0 0
         // 0 1 0 1 0
         // 0 1 1 1 0
         // 0 1 1 1 0
             for (int i=1; i<WIDTH-1; i++)
             {
                 // calculate the average
                 b = ( src[offs-1]           +                   + src[offs+1]
                     + src[offs+WIDTH-1]     + src[offs+WIDTH]   + src[offs+WIDTH+1]
                     + src[offs+(2*WIDTH)-1] + src[offs+WIDTH*2] + src[offs+(2*WIDTH)+1] ) >> 3;
                 // decrement the sum by one so that the fire looses intensity
                 if (b>0) b--;
                 // store the pixel
                 dst[offs] = b;
                 offs++;
             }
             // set last pixel of the line to 0
             dst[offs] = 0; offs++;
         }
        // clear the last 2 lines
        memset( (void *)((long)(dst)+offs), 0, 2*WIDTH );
    }

    bool OnUserCreate() override 
    {
        HEIGHT = ScreenHeight();
        WIDTH = ScreenWidth(); 

        current_time = 0;
        hotspot_width = WIDTH * 12;
        hotspot_start = WIDTH*HEIGHT - hotspot_width;

        palette = new u8[256*3];

        Shade_Pal( 0, 23, 0, 0, 0, 0, 0, 31 );
        Shade_Pal( 24, 47, 0, 0, 31, 63, 0, 0 );
        Shade_Pal( 48, 63, 63, 0, 0, 63, 63, 0 );
        Shade_Pal( 64, 127, 63, 63, 0, 63, 63, 63 );
        Shade_Pal( 128, 255, 63, 63, 63, 63, 63, 63 );

        fire1 = new u8[WIDTH*HEIGHT];
        fire2 = new u8[WIDTH*HEIGHT];

        // clear the buffers
        memset(fire1, 0, WIDTH*HEIGHT);
        memset(fire2, 0, WIDTH*HEIGHT);

        image = new olc::Sprite("image.png");
        return true;    
    }

    bool OnUserUpdate(float fElapsedTime) override 
    {
        current_time += fElapsedTime;


        // heat the fire
        Heat(fire1);
        // apply the filter
        Blur_Up(fire1, fire2);

        // draw
        int color_val, dst = 0;
        for(int y = 0; y < HEIGHT; y++) { 
            for(int x = 0; x < WIDTH; x++) { 
                color_val = (int)fire2[dst++];
                Draw(x, y, olc::Pixel(palette[color_val*3], 
                                      palette[color_val*3+1], 
                                      palette[color_val*3+2]));
            }
        }

        //DrawSprite(100, 100, image);

    // swap our two fire buffers
       tmp = fire1;
       fire1 = fire2;
       fire2 = tmp;

        return true;    
    }

    bool OnUserDestroy() override 
    {
        delete [] fire1;
        delete [] fire2;
        delete [] palette;
        delete image;
        return true;    
    }
};


int main () {

    Demo demo;
    if (demo.Construct(800, 600, 1, 1)) {
        demo.Start();
    }
    return 0;
}


