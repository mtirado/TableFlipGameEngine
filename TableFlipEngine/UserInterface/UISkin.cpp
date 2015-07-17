/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "UISkin.h"

UISkin::UISkin()
{
    //128x128 texture
    atlasRes = 128;

    //set all buffers to null
    memset(texcoords,   0, UI_SKIN_NUM * sizeof(GLuint));
    memset(info,        0, UI_SKIN_NUM * sizeof(void *));
    memset(numVerts,    0, UI_SKIN_NUM * sizeof(unsigned int));
    memset(atlasOrigin, 0, UI_SKIN_NUM * sizeof(Vector2)); 
    glGenBuffers(UI_SKIN_NUM, texcoords);

    //setup atlas offsets
    atlasOrigin[UI_SKIN_FRAME]     = Vector2( 0,    0 );
    atlasOrigin[UI_SKIN_SCROLLBAR] = Vector2( 0,   12 );
    atlasOrigin[UI_SKIN_BTN]       = Vector2( 36,   0 );
    atlasOrigin[UI_SKIN_BTN_HOVER] = Vector2( 48,   0 );
    atlasOrigin[UI_SKIN_BTN_DOWN]  = Vector2( 60,   0 ); 
    atlasOrigin[UI_SKIN_SLIDER]    = Vector2( 24,   0 );

    //color palletes
    atlasOrigin[UI_SKIN_LISTBOX]   = Vector2( 4,  127 );
    atlasOrigin[UI_SKIN_TEXTFIELD] = Vector2( 4,  126 );

    //this first run thing is basically an init
    //so i don't need a seperate function for that
    //sets up texcoords on first run, every run after will
    Vector2 zero(0.0);
    BuildFrame(zero,zero,0,0);
    BuildScrollbar(zero,zero,0.0,0,0);
    BuildSlider(zero,zero,0.0,0,0);
    BuildButton(zero,zero,UI_SKIN_BTN,0,0);
    BuildButton(zero,zero,UI_SKIN_BTN_HOVER,0,0);
    BuildButton(zero,zero,UI_SKIN_BTN_DOWN,0,0);
    BuildTextArea(zero,zero,UI_SKIN_TEXTFIELD,0,0);

    BuildListBox();

}

UISkin::~UISkin()
{
    Destroy();
}

void UISkin::Destroy()
{
    glDeleteBuffers(UI_SKIN_NUM, texcoords);
}


//builds a frame of quads around window (on window area)
void UISkin::BuildFrame(Vector2 position, Vector2 dimensions,
        Vector2 *out, unsigned int *outNum)
{
    //setup vert positions
    unsigned int verts = 8 * 4 - 2; //skip last 2 degenerates

    //this is where you grab the offset(ui frame has no offset)
    //Vector2 offset = atlasOrigin[UI_SKIN_FRAME];

    numVerts[UI_SKIN_FRAME] = verts; 
    float frameThickness = 4.0f;

    if (!out || !outNum)
    {
        //first run, setup static TC's
        /*  top left + top center
            2----4----------6   == vert order -- clockwise winding
            |\   | \        |
            | \  |    \     |
            |  \ |      \   |   *positions use top left origin
            |   \|        \ |
            1----3----------5  **texcoords use bottom left origin
            */ 

        //TODO i would really like to know if this gets optimized or not.
        //frame cell starts at 0,0 in atlas, no offset needed
        float cellSize = frameThickness / atlasRes;
        float dataTC[][2] = {
            //top left
            { 0.0, cellSize * 2.0 },
            { 0.0, cellSize * 3.0 },
            { cellSize, cellSize * 2.0 },
            { cellSize, cellSize * 3.0 },
            //top center
            { cellSize, cellSize * 2.0 },
            { cellSize, cellSize * 3.0 },
            //degenerates, not rasterized.
            { 0.0, 0.0 },
            { 0.0, 0.0 },
            //top right
            { cellSize * 2.0, cellSize * 3.0 },
            { cellSize * 3.0, cellSize * 3.0 },
            { cellSize * 2.0, cellSize * 2.0 },
            { cellSize * 3.0, cellSize * 2.0 },
            //top center
            { cellSize * 2.0, cellSize },
            { cellSize * 3.0, cellSize },
            //degenerates, not rasterized.
            { 0.0, 0.0 },
            { 0.0, 0.0 },
            //bottom right
            { cellSize * 3.0, cellSize },
            { cellSize * 3.0, 0.0 },
            { cellSize * 2.0, cellSize },
            { cellSize * 2.0, 0.0 },
            //bottom center
            { cellSize, cellSize },
            { cellSize, 0.0 },
            //degenerates, not rasterized.
            { 0.0, 0.0 },
            { 0.0, 0.0 },
            //bottom left
            { cellSize, 0.0 },
            { 0.0, 0.0 },
            { cellSize, cellSize },
            { 0.0, cellSize },
            //left center
            { cellSize, cellSize*2.0 },
            { 0.0, cellSize*2.0 },
            //degenerates, not rasterized.
            // { 0.0, 0.0 },
            // { 0.0, 0.0 },
            // ignore last 2 degenerates
        }; 


        glBindBuffer(GL_ARRAY_BUFFER, texcoords[UI_SKIN_FRAME]);
        glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
        glVertexAttribPointer(gUIManager->GetTexcoordLocation(), 2, 
                GL_FLOAT, GL_FALSE, 0,0);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*verts, 
                dataTC, GL_STATIC_DRAW);

        //first run sets up count and TC's

        //set frame thickness in pixels 
        info[UI_SKIN_FRAME].width  = frameThickness;
        info[UI_SKIN_FRAME].height = frameThickness;
        return;
    }


    //inner frame point
    float frameRight = position.x + dimensions.x - frameThickness;
    float frameLeft  = position.x + frameThickness;
    float frameTop   = position.y + frameThickness;
    float frameBtm   = position.y + dimensions.y - frameThickness;

    //outer frame point
    float rightMost  = position.x + dimensions.x;
    float bottomMost = position.y + dimensions.y;

    Vector2 data[] = {

        //these are top left oriented 

        //top left corner
        Vector2( position.x, frameTop   ),
        Vector2( position.x, position.y ),
        Vector2( frameLeft,  frameTop   ),
        Vector2( frameLeft,  position.y ),
        //top center
        Vector2( frameRight, frameTop   ),
        Vector2( frameRight, position.y ),
        //degenerates
        Vector2( frameRight, position.y ),
        Vector2( frameRight, position.y ),


        //top right
        Vector2( frameRight, position.y ),
        Vector2( rightMost,  position.y ),
        Vector2( frameRight, frameTop   ),
        Vector2( rightMost,  frameTop   ),
        //center right
        Vector2( frameRight, frameBtm   ),
        Vector2( rightMost,  frameBtm   ),
        //degenerates
        Vector2( rightMost,  frameBtm   ),
        Vector2( rightMost,  frameBtm   ),


        //bottom right
        Vector2( rightMost,  frameBtm   ),
        Vector2( rightMost,  bottomMost ),
        Vector2( frameRight, frameBtm   ),
        Vector2( frameRight, bottomMost ),
        //center bottom
        Vector2( frameLeft,  frameBtm   ),
        Vector2( frameLeft,  bottomMost ),
        //degenerates
        Vector2( frameLeft,  bottomMost ),
        Vector2( frameLeft,  bottomMost ),


        //bottom left
        Vector2( frameLeft,  bottomMost ),
        Vector2( position.x, bottomMost ),
        Vector2( frameLeft,  frameBtm   ),
        Vector2( position.x, frameBtm   ),
        //center left
        Vector2( frameLeft,  frameTop   ),
        Vector2( position.x, frameTop   ),
        //degenerates
        //Vector2( position.x, frameTop   ),
        //Vector2( position.x, frameTop   ),
        //ignore last 2 degenerates
    };

    memcpy(out, data, sizeof(Vector2) * verts); 
    *outNum = verts;
}


//builds a frame of quads around window (on window area)
void UISkin::BuildScrollbar(Vector2 position, Vector2 dimensions, 
        float scrollVal, Vector2 *out, unsigned int *outNum)
{
    //setup vert positions
    unsigned int verts = 22; //skip last 2 degenerates
    float width = 12.0;          
    bool vertical = dimensions.y > dimensions.x; 

    //this is where you grab the offset(ui frame has no offset)
    //Vector2 offset = atlasOrigin[UI_SKIN_SCROLLBAR];

    numVerts[UI_SKIN_SCROLLBAR] = verts; 
    if (!out || !outNum) //only do this once..
    {
        //first run, setup static TC's
        /* 
           ( cellsize * 5) 16v example(without scroll nub)
           1---2
           |   |
           3---4  5,6  --degenerate
           7---8  
           |   |
           |   |
           |   |
           9---10  11, 12
           13---14
           |   |
           15---16
           ( cellsize * 1 )

*/ 


        //how many pixels wide, this assumes a Square.
        info[UI_SKIN_SCROLLBAR].width = width;
        info[UI_SKIN_SCROLLBAR].height = width;


        //bottom scroll button's cell starts at y+12
        float cellSize = width / atlasRes;
        if (vertical)
        {
            float dataTC[][2] =  {

                //top scrollbar button
                { 0.0,            cellSize * 5.0 },
                { cellSize,       cellSize * 5.0 },
                { 0.0,            cellSize * 4.0 },
                { cellSize,       cellSize * 4.0 },
                //degenerates, not rasterized.
                { 0.0,            0.0 },
                { 0.0,            0.0 },

                //scroll background
                { 0.0,            cellSize * 4.0 },
                { cellSize,       cellSize * 4.0 },
                { 0.0,            cellSize * 3.0 },
                { cellSize,       cellSize * 3.0 },
                //degenerates, not rasterized.
                { 0.0,            0.0 },
                { 0.0,            0.0 },

                //scroll location nub
                { 0.0,            cellSize * 3.0 },
                { cellSize,       cellSize * 3.0 },
                { 0.0,            cellSize * 2.0 },
                { cellSize,       cellSize * 2.0 },
                //degenerates, not rasterized.
                { 0.0,            0.0 },
                { 0.0,            0.0 },


                //bottom scrollbar button
                { 0.0,            cellSize * 2.0 },
                { cellSize,       cellSize * 2.0 },
                { 0.0,            cellSize },
                { cellSize,       cellSize },

            };

            glBindBuffer(GL_ARRAY_BUFFER, texcoords[UI_SKIN_SCROLLBAR]);
            glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
            glVertexAttribPointer(gUIManager->GetTexcoordLocation(), 2, 
                    GL_FLOAT, GL_FALSE, 0,0);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*verts, 
                    dataTC, GL_STATIC_DRAW);
        }
        else //horizontal (texcoords rotated 90 degreed CCW) flips tc.x too
        {
            float dataTC[][2] =  {

                { 0.0,            cellSize * 5.0 },
                { 0.0,            cellSize * 4.0 },
                { cellSize,       cellSize * 5.0 },
                { cellSize,       cellSize * 4.0 },

                //degenerates, not rasterized.
                { 0.0,            0.0 },
                { 0.0,            0.0 },

                //background  (horizontal flips the x coords)
                { 0.0,       cellSize * 4.0 },
                { 0.0,       cellSize * 3.0 },
                { cellSize,  cellSize * 4.0 },
                { cellSize,  cellSize * 3.0 },
                //degenerates, not rasterized.
                { 0.0,            0.0 },
                { 0.0,            0.0 },

                //scroll location nub
                { 0.0,            cellSize * 3.0 },
                { 0.0,            cellSize * 2.0 },
                { cellSize,       cellSize * 3.0 },
                { cellSize,       cellSize * 2.0 },
                //degenerates, not rasterized.
                { 0.0,            0.0 },
                { 0.0,            0.0 },


                //bottom scrollbar button
                { 0.0,            cellSize * 2.0 },
                { 0.0,            cellSize },
                { cellSize,       cellSize * 2.0 },
                { cellSize,       cellSize },



            };

            glBindBuffer(GL_ARRAY_BUFFER, texcoords[UI_SKIN_SCROLLBAR]);
            glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
            glVertexAttribPointer(gUIManager->GetTexcoordLocation(), 2, 
                    GL_FLOAT, GL_FALSE, 0,0);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*verts, 
                    dataTC, GL_STATIC_DRAW);

        }
        //first run(init) only sets up count and TC's

        return;
    }

    float halfWidth = width/2.0;
    if (vertical)
    {
        float posXw  = position.x + width;
        float posYw  = position.y + width;
        float posYw2 = position.y + dimensions.y - width;
        float posYw3 = position.y + dimensions.y;
        float posYsw = position.y + scrollVal    + width;
        float posYs = position.y  + scrollVal;
        Vector2 data[] = {

            //these are top left oriented 
            //top button
            Vector2( position.x, position.y ),
            Vector2( posXw,      position.y ),
            Vector2( position.x, posYw      ),
            Vector2( posXw,      posYw      ),
            //degen
            Vector2( posXw,      posYw      ),
            Vector2( position.x, posYw      ),


            //scroll background
            Vector2( position.x, posYw      ),
            Vector2( posXw,      posYw      ),
            Vector2( position.x, posYw2     ),
            Vector2( posXw,      posYw2     ),
            //degen
            Vector2( posXw,      posYw2     ),
            Vector2( position.x, posYs-halfWidth      ),


            //change scrollbar location here:            
            //i'd rather have 1 draw call than just update a single quad
            //scrolling the window will invalidate all elements anyway..

            //scroll button
            Vector2( position.x, posYs-halfWidth  ),
            Vector2( posXw,      posYs-halfWidth  ),
            Vector2( position.x, posYsw-halfWidth ),
            Vector2( posXw,      posYsw-halfWidth ),
            //degen
            Vector2( posXw,      posYsw-halfWidth ),
            Vector2( position.x, posYw2 ),

            //bottom button
            Vector2( position.x, posYw2 ),
            Vector2( posXw,      posYw2 ),
            Vector2( position.x, posYw3 ),
            Vector2( posXw,      posYw3 ),
        };
        memcpy(out, data, sizeof(Vector2) * verts);
        *outNum = verts;

    }
    else //horizontal
    {
        float posYw  = position.y + width;
        float posXw  = position.x + width;
        float posXw2 = position.x + dimensions.x - width;
        float posXw3 = position.x + dimensions.x;
        float posXs  = position.x + scrollVal;
        float posXsw = position.x + scrollVal + width;
        Vector2 data[] = {

            //these are top left oriented 
            //top button
            Vector2( position.x,  position.y ),
            Vector2( posXw,       position.y ),
            Vector2( position.x,  posYw      ),
            Vector2( posXw,       posYw      ),
            //degen
            Vector2( posXw,       posYw      ),
            Vector2( posXw,       position.y ),


            //scroll background
            Vector2( posXw,  position.y ),
            Vector2( posXw2, position.y ),
            Vector2( posXw,  posYw      ),
            Vector2( posXw2, posYw      ),
            //degen
            Vector2( posXw2, posYw      ),
            Vector2( posXs-halfWidth,  position.y ),


            //change scrollbar location here:            
            //i'd rather have 1 draw call than just update a single quad
            //scrolling the window will invalidate all elements anyway..

            //scroll button
            Vector2( posXs-halfWidth,  position.y ),
            Vector2( posXsw-halfWidth, position.y ),
            Vector2( posXs-halfWidth,  posYw      ),
            Vector2( posXsw-halfWidth, posYw      ),
            //degen
            Vector2( posXsw-halfWidth, posYw      ),
            Vector2( posXw2, position.y ),


            //bottom button
            Vector2( posXw2, position.y ),
            Vector2( posXw3, position.y ),
            Vector2( posXw2, posYw      ),
            Vector2( posXw3, posYw      ),
        };
        memcpy(out, data, sizeof(Vector2) * verts);
        *outNum = verts;
    } 

}





//      SLIDER

///info width / height stores button dimensions in vertical orientation
///height = length
void UISkin::BuildSlider(Vector2 position, Vector2 dimensions, 
        float scrollVal, Vector2 *out, unsigned int *outNum)
{
    //setup vert positions
    unsigned int verts = 22; //skip last 2 degenerates

    float capLength = 4.0; //length of end caps (for clamping)
    float width     = 4.0;    
    float btnWidth  = 8.0;
    float btnHeight = 12.0;    
    bool  vertical = dimensions.y > dimensions.x; 

    //this is where you grab the offset(ui frame has no offset)
    Vector2 offset = atlasOrigin[UI_SKIN_SLIDER];

    numVerts[UI_SKIN_SLIDER] = verts; 
    if (!out || !outNum) //only do this once..
    {
        //first run, setup static TC's
        /* 
           ( cellsize * 5) 16v example(without scroll nub)
           1---2
           |   |
           3---4  5,6  --degenerate
           7---8  
           |   |
           |   |
           |   |
           9---10  11, 12
           13---14
           |   |
           15---16
           ( cellsize * 1 )

*/
        //set up info for button dimensions assumes vertical orientation
        info[UI_SKIN_SLIDER].width  = btnWidth;
        info[UI_SKIN_SLIDER].height = btnHeight;


        float x      = offset.x / atlasRes;
        float btnW = btnWidth/atlasRes;
        //bottom scroll button's cell starts at y+12
        float cellSize = width / atlasRes;
        if (vertical)
        {
            float dataTC[][2] =  {

                //top scrollbar button
                { x,            cellSize * 3.0 },
                { x+cellSize,   cellSize * 3.0 },
                { x,            cellSize * 2.0 },
                { x+cellSize,   cellSize * 2.0 },
                //degenerates, not rasterized.
                { 0.0,            0.0 },
                { 0.0,            0.0 },

                //scroll background
                { x,            cellSize * 2.0 },
                { x+cellSize,   cellSize * 2.0 },
                { x,            cellSize       },
                { x+cellSize,   cellSize       },
                //degenerates, not rasterized.
                { 0.0,            0.0 },
                { 0.0,            0.0 },

                //bottom scrollbar button
                { x,            cellSize },
                { x+cellSize,   cellSize },
                { x,            0.0 },
                { x+cellSize,   0.0 },

                //degenerates, not rasterized.
                { 0.0,            0.0 },
                { 0.0,            0.0 },

                //scroll location nub
                { x+cellSize,      btnHeight/atlasRes },
                { x+cellSize+btnW, btnHeight/atlasRes },
                { x+cellSize,      0.0                },
                { x+cellSize+btnW, 0.0                },


            };

            glBindBuffer(GL_ARRAY_BUFFER, texcoords[UI_SKIN_SLIDER]);
            glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
            glVertexAttribPointer(gUIManager->GetTexcoordLocation(), 2, 
                    GL_FLOAT, GL_FALSE, 0,0);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*verts, 
                    dataTC, GL_STATIC_DRAW);
        }
        else //horizontal (texcoords rotated 90 degreed CCW) flips tc.x too
        {
            float dataTC[][2] =  {

                { x,            cellSize * 3.0 },
                { x,            cellSize * 2.0 },
                { x+cellSize,   cellSize * 3.0 },
                { x+cellSize,   cellSize * 2.0 },

                //degenerates, not rasterized.
                { 0.0,            0.0 },
                { 0.0,            0.0 },

                //background  (horizontal flips the x coords)
                { x,           cellSize * 1.0 },
                { x,           cellSize * 2.0 },
                { x+cellSize,  cellSize * 1.0 },
                { x+cellSize,  cellSize * 2.0 },
                //degenerates, not rasterized.
                { 0.0,            0.0 },
                { 0.0,            0.0 },


                //bottom scrollbar button
                { x,            cellSize },
                { x,            0.0      },
                { x+cellSize,   cellSize },
                { x+cellSize,   0.0      },  

                //degenerates, not rasterized.
                { 0.0,            0.0 },
                { 0.0,            0.0 },

                //scroll location nub
                { x+cellSize,        btnHeight/atlasRes }, //NOTE height is hardcoded 12px
                { x+cellSize,        0.0 },
                { x+cellSize+btnW,   btnHeight/atlasRes },
                { x+cellSize+btnW,   0.0 },



            };

            glBindBuffer(GL_ARRAY_BUFFER, texcoords[UI_SKIN_SLIDER]);
            glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
            glVertexAttribPointer(gUIManager->GetTexcoordLocation(), 2, 
                    GL_FLOAT, GL_FALSE, 0,0);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*verts, 
                    dataTC, GL_STATIC_DRAW);

        }
        //first run sets up count and TC's

        //how many pixels wide, some things are assumed square
        //TODO?
        //info[UI_SKIN_SLIDER].width = width;
        //info[UI_SKIN_].height = width;

        return;
    }

    if (vertical)
    {
        //offset the controls so the button is fully within uielem rect
        position.x += btnWidth/4;
        float halfheight = btnHeight/2;
        float posXw  = position.x + width;
        float posYw  = position.y + width;
        float posYw2 = position.y + dimensions.y - width;
        float posYw3 = position.y + dimensions.y;
        float posYsw = position.y + scrollVal    + btnHeight;
        float posYs = position.y  + scrollVal;
        Vector2 data[] = {

            //these are top left oriented 
            //top button
            Vector2( position.x, position.y ),
            Vector2( posXw,      position.y ),
            Vector2( position.x, posYw      ),
            Vector2( posXw,      posYw      ),
            //degen
            Vector2( posXw,      posYw      ),
            Vector2( position.x, posYw      ),


            //scroll background
            Vector2( position.x,   posYw    ),
            Vector2( posXw,        posYw    ),
            Vector2( position.x,   posYw2   ),
            Vector2( posXw,        posYw2   ),
            //degen
            Vector2( posXw,        posYw2   ),
            Vector2( position.x,   posYw2 ),

            //bottom
            Vector2( position.x, posYw2 ),
            Vector2( posXw,      posYw2 ),
            Vector2( position.x, posYw3 ),
            Vector2( posXw,      posYw3 ),
            //degen
            Vector2( posXw,      posYw3 ),
            Vector2( position.x-2,              posYs-halfheight  ),

            //scroll button
            Vector2( position.x-2,              posYs-halfheight  ),
            Vector2( position.x+btnWidth-2,     posYs-halfheight  ),
            Vector2( position.x-2,              posYsw-halfheight ),
            Vector2( position.x+btnWidth-2,     posYsw-halfheight ),
        };
        memcpy(out, data, sizeof(Vector2) * verts);
        *outNum = verts;

    }
    else //horizontal
    {
        float halfWidth = btnHeight / 2;
        float posYw  = position.y + width;
        float posXw  = position.x + width;
        float posXw2 = position.x + dimensions.x - width;
        float posXw3 = position.x + dimensions.x;
        float posXs  = position.x + scrollVal;
        float posXsw = posXs + btnHeight; //hardcoded

        //artificially clamp position here, and adjust for it in slider math
        if (scrollVal-halfWidth <= 0.0)
        {
            posXs  = position.x + halfWidth;
            posXsw = posXs + btnHeight;
        }
        if (scrollVal+halfWidth >= dimensions.x)
        {
            posXs  = position.x + halfWidth + dimensions.x - btnHeight;
            posXsw = position.x + halfWidth + dimensions.x;
        }
        Vector2 data[] = {

            //these are top left oriented 
            //top button
            Vector2( position.x,  position.y ),
            Vector2( posXw,       position.y ),
            Vector2( position.x,  posYw      ),
            Vector2( posXw,       posYw      ),
            //degen
            Vector2( posXw,       posYw      ),
            Vector2( posXw,       position.y ),


            //scroll background
            Vector2( posXw,  position.y ),
            Vector2( posXw2, position.y ),
            Vector2( posXw,  posYw      ),
            Vector2( posXw2, posYw      ),
            //degen
            Vector2( posXw2, posYw      ),
            Vector2( posXw2, position.y ),


            //bottom
            Vector2( posXw2, position.y ),
            Vector2( posXw3, position.y ),
            Vector2( posXw2, posYw      ),
            Vector2( posXw3, posYw      ),

            Vector2( posXw3, posYw      ),
            Vector2( posXs-halfWidth,  position.y-2 ),
            //change scrollbar location here:            
            //i'd rather have 1 draw call than just update a single quad
            //scrolling the window will invalidate all elements anyway..

            Vector2( posXs-halfWidth,  position.y-2 ),
            Vector2( posXsw-halfWidth, position.y-2 ),
            Vector2( posXs-halfWidth,  position.y+btnWidth-2      ),
            Vector2( posXsw-halfWidth, position.y+btnWidth-2      ),

        };
        memcpy(out, data, sizeof(Vector2) * verts);
        *outNum = verts;
    } 

}









void UISkin::BuildListBox()
{
    //setup vert positions
    unsigned int verts = 4;
    float texWidth     = 4.0; //4 pixels wide in atlas

    //this is where you grab the offset(ui frame has no offset)
    //Vector2 offset = atlasOrigin[UI_SKIN_FRAME];

    numVerts[UI_SKIN_LISTBOX] = verts; 
    //first run, setup static TC's
    /*  top left + top center
        2--------4  == vert order -- clockwise winding
        | \      |
        |   \    |
        |     \  |   *positions use top left origin
        |       \|
        1--------3  **texcoords use bottom left origin
        */ 

    //frame cell starts at 0,0 in atlas, no offset needed
    float cellSize = texWidth / atlasRes;
    float dataTC[][2] =  {

        //top left 
        { 0.0,            cellSize * 2.0 },
        { 0.0,            cellSize * 3.0 },
        { cellSize,       cellSize * 2.0 },
        { cellSize,       cellSize * 3.0 },
    };

    glBindBuffer(GL_ARRAY_BUFFER, texcoords[UI_SKIN_LISTBOX]);
    glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
    glVertexAttribPointer(gUIManager->GetTexcoordLocation(), 2, 
            GL_FLOAT, GL_FALSE, 0,0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*verts, 
            dataTC, GL_STATIC_DRAW);
}







void UISkin::BuildTextArea(Vector2 position, Vector2 dimensions,
        UISkinnable elem, Vector2 *out, unsigned int *outNum)
{
    unsigned int verts = 10;
    float frameWidth   = 2.0;

    
    if (!out || !outNum) //only do this once..
    {
        numVerts[elem] = verts;
        Vector2 frame = atlasOrigin[elem] / atlasRes;
        Vector2 content = atlasOrigin[elem];
        content.x += 1.0;
        content /= atlasRes;
        //first run, setup static TC's
        /*  top left + top center
            2--------4  == vert order -- clockwise winding
            | \      |
            |   \    |
            |     \  |   *positions use top left origin
            |       \|
            1--------3  **texcoords use bottom left origin
            */ 

        //frame cell starts at 0,0 in atlas, no offset needed
        float dataTC[][2] =  {

            //outter border color
            { frame.x, frame.y },
            { frame.x, frame.y },
            { frame.x, frame.y },
            { frame.x, frame.y },

            //degenerates
            { frame.x,   frame.y   },
            { content.x, content.y },

            //inner content color
            { content.x, content.y },
            { content.x, content.y },
            { content.x, content.y },
            { content.x, content.y },

        };

        glBindBuffer(GL_ARRAY_BUFFER, texcoords[elem]);
        glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
        glVertexAttribPointer(gUIManager->GetTexcoordLocation(), 2, 
                GL_FLOAT, GL_FALSE, 0,0);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*verts, 
                dataTC, GL_STATIC_DRAW);

        info[elem].width  = frameWidth;
        info[elem].height = frameWidth;

        return;
    }


    float posXw  = position.x + dimensions.x;
    float posYw  = position.y + dimensions.y;
    float posXf  = position.x + frameWidth;
    float posXfw = posXw      - frameWidth;
    float posYf  = position.y + frameWidth;
    float posYfw = posYw      - frameWidth;

    Vector2 data[] = {

        //these are top left oriented 
        //first rect is the large one for frame color
        //top button
        Vector2( position.x, posYw      ),
        Vector2( position.x, position.y ),
        Vector2( posXw,      posYw      ),
        Vector2( posXw,      position.y ),
        //degen
        Vector2( posXw,      position.y ),
        Vector2( posXf,      posYf      ),

        //inner content area
        Vector2( posXf,  posYfw ),
        Vector2( posXf,  posYf  ),
        Vector2( posXfw, posYfw ),
        Vector2( posXfw, posYf  ),
    };
    memcpy(out, data, sizeof(Vector2) * verts);
    *outNum = verts;

}





///builds a button, atlasOffset is bottom left origin in texture atlas
void UISkin::BuildButton(Vector2 position, Vector2 dimensions,
        UISkinnable elem, Vector2 *out, unsigned int *outNum)
{
    //setup vert positions (its same as frame + background area too)
    unsigned int verts = 36; //skip last 2 degenerates

    //this is where you grab the offset(ui frame has no offset)
    Vector2 atlasOffset = atlasOrigin[elem] / atlasRes;

    numVerts[elem] = verts; 
    float frameThickness = 2.0f;
    float gridSize = 2.0f;
    if (!out || !outNum)
    {
        //first run, setup static TC's
        /*  top left + top center
            2----4----------6   == vert order -- clockwise winding
            |\   | \        |
            | \  |    \     |
            |  \ |      \   |   *positions use top left origin
            |   \|        \ |
            1----3----------5  **texcoords use bottom left origin
            */ 

        //TODO i would really like to know if this gets optimized or not.
        //frame cell starts at 0,0 in atlas, no offset needed
        float cellSize = gridSize / atlasRes;
        float cell5 = cellSize * 5.0;
        float cell6 = cellSize * 6.0;
        float dataTC[][2] =  {

            //top left 
            { atlasOffset.x,            atlasOffset.y + cell5 },
            { atlasOffset.x,            atlasOffset.y + cell6 },
            { atlasOffset.x + cellSize, atlasOffset.y + cell5 },
            { atlasOffset.x + cellSize, atlasOffset.y + cell6 },
            //top center
            { atlasOffset.x + cellSize, atlasOffset.y + cell5 },
            { atlasOffset.x + cellSize, atlasOffset.y + cell6 },
            //degenerates, not rasterized.
            { 0.0,            0.0 },
            { 0.0,            0.0 },

            //top right
            { atlasOffset.x + cell5, atlasOffset.y + cell6 },
            { atlasOffset.x + cell6, atlasOffset.y + cell6 },
            { atlasOffset.x + cell5, atlasOffset.y + cell5 },
            { atlasOffset.x + cell6, atlasOffset.y + cell5 },
            //top center
            { atlasOffset.x + cell5, atlasOffset.y + cellSize       },
            { atlasOffset.x + cell6, atlasOffset.y + cellSize       },
            //degenerates, not rasterized.
            { 0.0,            0.0 },
            { 0.0,            0.0 },

            //bottom right
            { atlasOffset.x + cell6, atlasOffset.y + cellSize },
            { atlasOffset.x + cell6, atlasOffset.y            },
            { atlasOffset.x + cell5, atlasOffset.y + cellSize },
            { atlasOffset.x + cell5, atlasOffset.y            },
            //bottom center
            { atlasOffset.x + cellSize,       atlasOffset.y + cellSize },
            { atlasOffset.x + cellSize,       atlasOffset.y            },
            //degenerates, not rasterized.
            { 0.0,            0.0 },
            { 0.0,            0.0 },

            //bottom left
            { atlasOffset.x + cellSize, atlasOffset.y            },
            { atlasOffset.x,            atlasOffset.y            },
            { atlasOffset.x + cellSize, atlasOffset.y + cellSize },
            { atlasOffset.x,            atlasOffset.y + cellSize },
            //left center
            { atlasOffset.x + cellSize, atlasOffset.y + cell5 },
            { atlasOffset.x,            atlasOffset.y + cell5 },
            //degenerates, not rasterized.
            { 0.0,            0.0 },
            { 0.0,            0.0 },

            //background -- x repeeat
            { atlasOffset.x + cellSize, atlasOffset.y + cellSize },
            { atlasOffset.x + cellSize, atlasOffset.y + cell5  },
            { atlasOffset.x + cellSize, atlasOffset.y + cellSize },
            { atlasOffset.x + cellSize, atlasOffset.y + cell5  },




        };

        glBindBuffer(GL_ARRAY_BUFFER, texcoords[elem]);
        glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
        glVertexAttribPointer(gUIManager->GetTexcoordLocation(), 2, 
                GL_FLOAT, GL_FALSE, 0,0);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*verts, 
                dataTC, GL_STATIC_DRAW);

        //first run sets up count and TC's

        //set frame thickness in pixels 
        info[elem].width  = frameThickness;
        info[elem].height = frameThickness;
        return;
    }

    //inner frame point
    float frameRight = position.x + dimensions.x - frameThickness;
    float frameLeft  = position.x + frameThickness;
    float frameTop   = position.y + frameThickness; //grrr needs a hunneth extra for perfect
    float frameBtm   = position.y + dimensions.y - frameThickness + 0.01; //horizontal lines

    /*LOGOUTPUT   << "--frame--\n"
      << "bottom : " << frameBtm << "\n"
      << "top    : " << frameTop << "\n"
      << "right  : " << frameRight << "\n"
      << "left   : " << frameLeft << "\n";
      LogInfo();*/
    //outer frame point
    float rightMost  = position.x + dimensions.x;
    float bottomMost = position.y + dimensions.y;

    Vector2 data[] = {

        //these are top left oriented 

        //top left corner
        Vector2( position.x, frameTop   ),
        Vector2( position.x, position.y ),
        Vector2( frameLeft,  frameTop   ),
        Vector2( frameLeft,  position.y ),
        //top center
        Vector2( frameRight, frameTop   ),
        Vector2( frameRight, position.y ),
        //degenerates
        Vector2( frameRight, position.y ),
        Vector2( frameRight, position.y ),


        //top right
        Vector2( frameRight, position.y ),
        Vector2( rightMost,  position.y ),
        Vector2( frameRight, frameTop   ),
        Vector2( rightMost,  frameTop   ),
        //center right
        Vector2( frameRight, frameBtm   ),
        Vector2( rightMost,  frameBtm   ),
        //degenerates
        Vector2( rightMost,  frameBtm   ),
        Vector2( rightMost,  frameBtm   ),


        //bottom right
        Vector2( rightMost,  frameBtm   ),
        Vector2( rightMost,  bottomMost ),
        Vector2( frameRight, frameBtm   ),
        Vector2( frameRight, bottomMost ),
        //center bottom
        Vector2( frameLeft,  frameBtm   ),
        Vector2( frameLeft,  bottomMost ),
        //degenerates
        Vector2( frameLeft,  bottomMost ),
        Vector2( frameLeft,  bottomMost ),


        //bottom left
        Vector2( frameLeft,  bottomMost ),
        Vector2( position.x, bottomMost ),
        Vector2( frameLeft,  frameBtm   ),
        Vector2( position.x, frameBtm   ),
        //center left
        Vector2( frameLeft,  frameTop   ),
        Vector2( position.x, frameTop   ),
        //degenerates
        Vector2( position.x, frameTop   ),
        //Vector2( position.x, frameTop   ),
        Vector2( frameLeft,  frameBtm   ),

        //bottom left
        Vector2( frameLeft,  frameBtm   ),
        Vector2( frameLeft,  frameTop   ),
        Vector2( frameRight, frameBtm   ),
        Vector2( frameRight, frameTop   ),

        //ignore last 2 degenerates
    };

    memcpy(out, data, sizeof(Vector2) * verts); 
    *outNum = verts;
}



