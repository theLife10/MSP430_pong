/** \file shapemotion.c
 *  \brief This is a simple shape motion demo.
 *  This demo creates two layers containing shapes.
 *  One layer contains a rectangle and the other a circle.
 *  While the CPU is running the green LED is on, and
 *  when the screen does not need to be redrawn the CPU
 *  is turned off along with the green LED.
 */  
#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include "buzzer.h"

#define GREEN_LED BIT6

Region fencePaddle1;
Region fencePaddle2;
Region fieldFence;
AbRect paddle = {abRectGetBounds, abRectCheck, {2,14}}; /**< 10x10 rectangle */

AbRectOutline fieldOutline = {	/* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,   
  {screenWidth/2 - 5, screenHeight/2 - 11}
};

//field
Layer fieldLayer = {		/* playing field as a layer */
  (AbShape *) &fieldOutline,
  {screenWidth/2, screenHeight/2},/**< center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_WHITE,
  0,
};
  
//paddle
Layer layer2 = {		
  (AbShape *)&paddle,
  {118, screenHeight / 2}, 
  {0,0}, {0,0},				    
  COLOR_VIOLET,
  &fieldLayer
};


//paddle
Layer layer1 = {		
  (AbShape *)&paddle,
  {10, screenHeight/2}, 
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_RED,
  &layer2,
};
//ball
Layer layer0 = {		/**< Layer with an orange circle */
  (AbShape *)&circle4,
  {screenWidth/2, screenHeight/2}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_ORANGE,
  &layer1,
};

/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */
typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

/* initial value of {0,0} will be overwritten */
MovLayer ml2 = { &layer2, {0,2}, 0 }; // paddle
MovLayer ml1 = { &layer1, {0,2}, 0 }; // paddle
MovLayer ml0 = { &layer0, {2,4}, 0 };  //ball

void movLayerDraw(MovLayer *movLayers, Layer *layers)
{
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);			/**< disable interrupts (GIE off) */
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8);			/**< disable interrupts (GIE on) */


  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1], 
		bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
	Vec2 pixelPos = {col, row};
	u_int color = bgColor;
	Layer *probeLayer;
	for (probeLayer = layers; probeLayer; 
	     probeLayer = probeLayer->next) { /* probe all layers, in order */
	  if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
	    color = probeLayer->color;
	    break; 
	  } /* if probe check */
	} // for checking all layers at col, row
	lcd_writeColor(color); 
      } // for col
    } // for row
  } // for moving layer being updated
}	  



Region fence = {{10,30}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}}; /**< Create a fence region */

/** Advances a moving shape within a fence
 *  
 *  \param ml The moving shape to be advanced
 *  \param fence The region which will serve as a boundary for ml
 */
void paddleCollision(MovLayer *ml, Region *fence)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  int velocity;
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 0; axis < 2; axis ++) {
      if ( (shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
	  (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]) )
	  {
          buzzer_set_period(100);
        int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
        newPos.axes[axis] += (1*velocity);
      }	/**< if outside of fence */
      
      
    } /**< for axis */
    
    
    
    ml->layer->posNext = newPos;
  } /**< for ml */
}

u_char scoredPaddle1 = '0';
u_char scoredPaddle2 = '0' ;
void game_fences(MovLayer *ml, Region *player1fence, Region *player2fence, Region *fence){
     
     
    Vec2 newPos;
    
    u_char axis;
    
    Region shapeBoundary;
    
    vec2Add( &newPos, &ml -> layer -> posNext, &ml -> velocity);
    abShapeGetBounds( ml -> layer -> abShape, &newPos, &shapeBoundary ) ;
    
    

    
    
     if(((shapeBoundary.topLeft.axes[0] <= player1fence -> botRight.axes[0]) &&
        (shapeBoundary.topLeft.axes[1] > player1fence -> topLeft.axes[1]) &&
        (shapeBoundary.topLeft.axes[1] < player1fence -> botRight.axes[1]))
         ) 
    {
        
        int velocity = ml -> velocity.axes[axis] = -ml -> velocity.axes[0];
        
        newPos.axes[0]  += ( 1 * velocity ) ;
         buzzer_set_period(200);
        
        
    }
   
    if( ( ( shapeBoundary.botRight.axes[ 0 ] >= player2fence -> topLeft.axes[ 0 ] ) )&&
        ( shapeBoundary.botRight.axes[ 1 ] > player2fence -> topLeft.axes[ 1] ) &&
        ( shapeBoundary.botRight.axes[ 1 ] < player2fence -> botRight.axes[ 1 ] ) ) {
           
            int velocity = ml -> velocity.axes[axis] = -ml -> velocity.axes[0];
         
            newPos.axes[0] += (1 * velocity ) ;
             buzzer_set_period(2000);
    }
    
    
    
    //ball hits to the other side then center the ball
    if( shapeBoundary.topLeft.axes[ 0 ] < fence -> topLeft.axes[0] 
        
    ) {
           buzzer_set_period(500);
            newPos.axes[0] = screenWidth / 2 ;
            newPos.axes[1] = screenHeight / 2;
            
            scoredPaddle2 = scoredPaddle2 - 255 ;
             
           
    }
    //ball hits to the other side then center the ball
    if( shapeBoundary.botRight.axes[ 0 ] > fence -> botRight.axes[0] ) {
        buzzer_set_period(500);
        newPos.axes[0] = screenWidth / 2 ;
        newPos.axes[1] = screenHeight / 2;
        
        scoredPaddle1 = scoredPaddle1 - 255 ;
    }
    
   
    if( (shapeBoundary.topLeft.axes[1] <= fence -> topLeft.axes[1]) ||
        (shapeBoundary.botRight.axes[1] >= fence ->botRight.axes[1] )){
            
        
        int velocity = ml -> velocity.axes[1] = -ml -> velocity.axes[1];
        newPos.axes[1] += (1*velocity);
        buzzer_set_period(400);
    }
    
    
    
    ml -> layer -> posNext = newPos ; 
  
    
}


u_int bgColor = COLOR_GREEN;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */

Region fieldFence;		/**< fence around playing field  */



/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */
void main()
{
  P1DIR |= GREEN_LED;		/**< Green led on when CPU on */		
  P1OUT |= GREEN_LED;

  configureClocks();
  lcd_init();
  shapeInit();
  p2sw_init(15);
  buzzer_init();

  shapeInit();

  layerInit(&layer1);
  layerDraw(&layer1);


  layerGetBounds(&fieldLayer, &fieldFence);
 
  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */
   
   
  for(;;) { 
    while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~GREEN_LED;    /**< Green led off witHo CPU */
      or_sr(0x10);	      /**< CPU OFF */
    }
    
    
    
    P1OUT |= GREEN_LED;       /**< Green led on when CPU on */
    redrawScreen = 0;
    
   
    
     drawChar5x7(30,150, scoredPaddle1, COLOR_VIOLET, COLOR_WHITE);
     drawChar5x7(100,150, scoredPaddle2, COLOR_VIOLET, COLOR_WHITE);
  }
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */

void wdt_c_handler()
{
  
  
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count ++;
  u_int switches = p2sw_read() , i;
  if (count == 10) {
 
    
    layerGetBounds(&layer1,&fencePaddle1);
    layerGetBounds(&layer2, &fencePaddle2);
    layerGetBounds(&fieldLayer, &fieldFence);
    
      movLayerDraw(&ml0, &layer0);
      
      game_fences( &ml0, &fencePaddle1, &fencePaddle2, &fieldFence) ;
      
      
     char botton [5];
     
     for( i = 0 ; i < 4 ; i++ ) {
        botton[i] = (switches & (1 << i ) ) ? 0 : 1 ;
     }
     
     botton [ 4 ] = 0 ;
     switch_one();
     if( botton [ 0 ] ) {
        ml1.velocity.axes[1] = -7 ;
       
       paddleCollision(&ml1, &fieldFence);
        
       movLayerDraw(&ml1, &layer1);
       
        
        redrawScreen = 1;
         
     }
     if ( botton[ 1 ] ) {
        ml1.velocity.axes[1] = 7;
      
       
        paddleCollision(&ml1, &fieldFence);
        
        movLayerDraw(&ml1, &layer1);
    
        
        redrawScreen =1;
         
     }
     if( botton[2] ) {
        ml2.velocity.axes[1] = 7 ;
       
      
        paddleCollision(&ml2 , &fieldFence );
        movLayerDraw(&ml2, &layer2);
         redrawScreen = 1;
     }
     if( botton[3] ) {
        ml2.velocity.axes[1] = -7;
       
       
        paddleCollision(&ml2, &fieldFence);
        movLayerDraw(&ml2, &layer2);
         redrawScreen = 1;
     }
     
     
     
    
    
    count = 0;
  } 

     if( scoredPaddle1 == '9' ){
          drawString5x7(30,2, "winner player 1", COLOR_VIOLET, COLOR_WHITE);
          switch_one();
        
     }
  
     if( scoredPaddle2 == '9' ){
          drawString5x7(30,2, "winner player 2", COLOR_VIOLET, COLOR_WHITE);
         
     }
    
    
    if(p2sw_read()){
        redrawScreen=1;
    }

  P1OUT &= ~GREEN_LED;	/**< Green LED off when cpu off */

   
}


