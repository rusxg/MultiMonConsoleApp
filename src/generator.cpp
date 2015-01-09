#include <windows.h>
#include <assert.h>
#include "generator.h"


PictureGenerator::PictureGenerator(int iImageWidth, int iImageHeight, int iBallSize) :
    m_iImageWidth(iImageWidth),
    m_iImageHeight(iImageHeight),
    m_iBallSize(iBallSize),
    m_iAvailableWidth(iImageWidth - iBallSize),
    m_iAvailableHeight(iImageHeight - iBallSize),
    m_x(0),
    m_y(0),
    m_xDir(RIGHT),
    m_yDir(UP)
{
    // Check we have some (arbitrary) space to bounce in.
    assert(iImageWidth > 2*iBallSize);
    assert(iImageHeight > 2*iBallSize);

    // Random position for showing off a video mixer
    m_iRandX = rand();
    m_iRandY = rand();

} // (Constructor)


void PictureGenerator::PlotBall(BYTE pFrame[], const BYTE BallPixel[], int iPixelSize)
{
    assert(m_x >= 0);
    assert(m_x <= m_iAvailableWidth);
    assert(m_y >= 0);
    assert(m_y <= m_iAvailableHeight);
    assert(pFrame != NULL);
    assert(BallPixel != NULL);

    // The current byte of interest in the frame
    BYTE *pBack;
    pBack = pFrame;     

    // Plot the ball into the correct location
    BYTE *pBall = pFrame + ( m_y * m_iImageWidth * iPixelSize) + m_x * iPixelSize;

    for(int row = 0; row < m_iBallSize; row++)
    {
        for(int col = 0; col < m_iBallSize; col++)
        {
            // For each byte fill its value from BallPixel[]
            for(int i = 0; i < iPixelSize; i++)
            {  
                if(WithinCircle(col, row))
                {
                    *pBall = BallPixel[i];
                }
                pBall++;
            }
        }
        pBall += m_iAvailableWidth * iPixelSize;
    }

} // PlotBall


int PictureGenerator::BallPosition(int iPixelTime, // Millisecs per pixel
                        int iLength,    // Distance between the bounce points
                        int time,       // Time in millisecs
                        int iOffset)    // For a bit of randomness
{
    // Calculate the position of an unconstrained ball (no walls)
    // then fold it back and forth to calculate the actual position

    int x = time / iPixelTime;
    x += iOffset;
    x %= 2 * iLength;

    // check it is still in bounds
    if(x > iLength)
    {    
        x = 2*iLength - x;
    }
    return x;

} // BallPosition


void PictureGenerator::MoveBall(LONG ms)
{
    m_x = BallPosition(10, m_iAvailableWidth, ms, m_iRandX);
    m_y = BallPosition(10, m_iAvailableHeight, ms, m_iRandY);

} // MoveBall


inline BOOL PictureGenerator::WithinCircle(int x, int y)
{
    unsigned int r = m_iBallSize / 2;

    if((x-r)*(x-r) + (y-r)*(y-r)  < r*r)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

} // WithinCircle


