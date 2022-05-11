/*
 * Group 8 - Bego Giacomo - Longo Mattia - 2022/05
 *
 * Assignment 3 - Vending Machine
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <sys/printk.h>
#include <sys/__assert.h>
#include <string.h>
#include <timing/timing.h>
#include <stdio.h>

/*Global variables*/
static int8_t sel_prod=1; /*Kind of products*/
static int credit=0; /*Credit available*/

/*Price of products*/
const int beerCost=150;
const int tunaSandwichCost=100;
const int coffeeCost=50;

#define SLEEP_MS 5 /* Blink period in ms*/ 

/*Define of states machine*/
#define IDLE 0
#define BROWSE_UP 1
#define BROWSE_DOWN 2
#define DISPENSING 3
#define RETURNING 4
#define CENT10 5
#define CENT20 6
#define CENT50 7
#define CENT100 8

#define COMPARISON 1
#define ERROR 2
#define OK 3
#define DISPENSE 4

/* Refer to dts file */
#define GPIO0_NID DT_NODELABEL(gpio0) 
#define BOARDBUT1 0xb /* Pin at which BUT1 is connected. Addressing is direct (i.e., pin number) */
#define BOARDBUT2 0xc /* Pin at which BUT2 is connected. Addressing is direct (i.e., pin number) */
#define BOARDBUT3 0x18 /* Pin at which BUT3 is connected. Addressing is direct (i.e., pin number) */
#define BOARDBUT4 0x19 /* Pin at which BUT4 is connected. Addressing is direct (i.e., pin number) */

#define BOARDBUT5 0x3 /* Pin at which BUT5 is connected. Addressing is direct (i.e., pin number) */
#define BOARDBUT6 0x4 /* Pin at which BUT6 is connected. Addressing is direct (i.e., pin number) */
#define BOARDBUT7 0x1C /* Pin at which BUT7 is connected. Addressing is direct (i.e., pin number) */
#define BOARDBUT8 0x1D /* Pin at which BUT8 is connected. Addressing is direct (i.e., pin number) */


/* Int related declarations */
static struct gpio_callback but1_cb_data; /* Callback structure */
static struct gpio_callback but2_cb_data; /* Callback structure */
static struct gpio_callback but3_cb_data; /* Callback structure */
static struct gpio_callback but4_cb_data; /* Callback structure */
static struct gpio_callback but5_cb_data; /* Callback structure */
static struct gpio_callback but6_cb_data; /* Callback structure */
static struct gpio_callback but7_cb_data; /* Callback structure */
static struct gpio_callback but8_cb_data; /* Callback structure */

/* Callback function and variables*/
volatile bool up = 0; /* Flag to signal a BUT1 press */
volatile bool down = 0; /* Flag to signal a BUT2 press */
volatile bool select = 0; /* Flag to signal a BUT3 press */
volatile bool c_return = 0; /* Flag to signal a BUT4 press */
volatile bool c10 = 0; /* Flag to signal a BUT5 press */
volatile bool c20 = 0; /* Flag to signal a BUT6 press */
volatile bool c50 = 0; /* Flag to signal a BUT7 press */
volatile bool c100 = 0; /* Flag to signal a BUT8 press */

void but1press_cbfunction(){
    //printk("UP product\n"); // button 1 hit !    
    up = 1;
}
void but2press_cbfunction(){
    //printk("DOWN product\n"); // button 2 hit !    
    down = 1;
}
void but3press_cbfunction(){
    //printk("PRODUCT SELECTED\n"); // button 3 hit !    
    select = 1;
}
void but4press_cbfunction(){
    //printk("RETURNING REQUEST ACTIVATED\n"); // button 4 hit !    
    c_return = 1;
}
void but5press_cbfunction(){
    //printk("10cent inserted\n"); // button 4 hit !    
    c10 = 1;
}
void but6press_cbfunction(){
    //printk("20 cent inserted\n"); // button 4 hit !    
    c20 = 1;
}
void but7press_cbfunction(){
    //printk("50cent inserted\n"); // button 4 hit !    
    c50 = 1;
}
void but8press_cbfunction(){
    //printk("1 euro inserted \n"); // button 4 hit !    
    c100 = 1;
}

void main(void) {

    /* Local vars */
    const struct device *gpio0_dev;         /* Pointer to GPIO device structure */

    int ret=0; 

    gpio0_dev = device_get_binding(DT_LABEL(GPIO0_NID));
    
    //check the buttons binding
    if (gpio0_dev == NULL) {
        printk("Error: Failed to bind to GPIO0\n\r");        
	return;
    }
    else {
        printk("Bind to GPIO0 successfull \n\r");        
    }
    
    //configuring buttons
    
    ret = gpio_pin_configure(gpio0_dev, BOARDBUT1, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printk("Error %d: Failed to configure BUT 1 \n\r", ret);
	return;
    }
    ret = gpio_pin_configure(gpio0_dev, BOARDBUT2, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printk("Error %d: Failed to configure BUT 2 \n\r", ret);
	return;
    }
    ret = gpio_pin_configure(gpio0_dev, BOARDBUT3, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printk("Error %d: Failed to configure BUT 3 \n\r", ret);
	return;
    }
    ret = gpio_pin_configure(gpio0_dev, BOARDBUT4, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printk("Error %d: Failed to configure BUT 4 \n\r", ret);
	return;
    }
  
  ret = gpio_pin_configure(gpio0_dev, BOARDBUT5, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printk("Error %d: Failed to configure BUT 5 \n\r", ret);
	return;
    }
    ret = gpio_pin_configure(gpio0_dev, BOARDBUT6, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printk("Error %d: Failed to configure BUT 6 \n\r", ret);
	return;
    }
    ret = gpio_pin_configure(gpio0_dev, BOARDBUT7, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printk("Error %d: Failed to configure BUT 7 \n\r", ret);
	return;
    }
    ret = gpio_pin_configure(gpio0_dev, BOARDBUT8, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printk("Error %d: Failed to configure BUT 8 \n\r", ret);
	return;
    }
  
    /* Set interrupt HW - which pin and event generate interrupt */
    ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT1, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&but1_cb_data, but1press_cbfunction, BIT(BOARDBUT1));
    gpio_add_callback(gpio0_dev, &but1_cb_data);

    ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT2, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&but2_cb_data, but2press_cbfunction, BIT(BOARDBUT2));
    gpio_add_callback(gpio0_dev, &but2_cb_data);

    ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT3, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&but3_cb_data, but3press_cbfunction, BIT(BOARDBUT3));
    gpio_add_callback(gpio0_dev, &but3_cb_data);

    ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT4, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&but4_cb_data, but4press_cbfunction, BIT(BOARDBUT4));
    gpio_add_callback(gpio0_dev, &but4_cb_data);
    
    ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT5, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&but5_cb_data, but5press_cbfunction, BIT(BOARDBUT5));
    gpio_add_callback(gpio0_dev, &but5_cb_data);

    ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT6, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&but6_cb_data, but6press_cbfunction, BIT(BOARDBUT6));
    gpio_add_callback(gpio0_dev, &but6_cb_data);

    ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT7, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&but7_cb_data, but7press_cbfunction, BIT(BOARDBUT7));
    gpio_add_callback(gpio0_dev, &but7_cb_data);

    ret = gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT8, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&but8_cb_data, but8press_cbfunction, BIT(BOARDBUT8));
    gpio_add_callback(gpio0_dev, &but8_cb_data);
    
    int state=IDLE;
  
      while(1){
    switch(state){
      case IDLE:
        if(up==1) { state=BROWSE_UP; }
        if(down==1) { state=BROWSE_DOWN; }
        if(select==1) { state=DISPENSING; }
        if(c10==1) {state=CENT10; }
        if(c20==1) {state=CENT20; }
        if(c50==1) {state=CENT50; }
        if(c100==1) {state=CENT100; }
        if(c_return==1) { state=RETURNING; }
      break;
      
      case BROWSE_UP:
        up=0;
        if(sel_prod<3){
            sel_prod=sel_prod+1;
        }
        /*Print the product and */
        if(sel_prod==1){
    	printk("Beer: 1.50 EUR\n");
  	}
  	else if(sel_prod==2){
    	printk("Tuna sandwitch: 1.00 EUR\n");
  	}
  	else if(sel_prod==3){
    	printk("Coffee: 0.50 EUR\n");
  	}
  	printk("Credit: %d\n\r",credit);
        state=IDLE;
      break;

      case BROWSE_DOWN:
      down=0;
        if(sel_prod>1){
            sel_prod=sel_prod-1;
        }
        /*Print the product and */
        if(sel_prod==1){
    	printk("Beer: 1.50 EUR\n");
  	}
  	else if(sel_prod==2){
    	printk("Tuna sandwitch: 1.00 EUR\n");
  	}
  	else if(sel_prod==3){
    	printk("Coffee: 0.50 EUR\n");
  	}
  	printk("Credit: %d\n\r",credit);
        state=IDLE;
      break;

      case DISPENSING:
        select=0;
        dispensing_superstate();
        state=IDLE;
      break;

      case CENT10:
      c10=0;
      	credit=credit+10;
      	printk("Credit: %d\n\r",credit);
      	state=IDLE;
      break;
      
      case CENT20:
      c20=0;
      	credit=credit+20;
      	printk("Credit: %d\n\r",credit);
      	state=IDLE;
      break;
      
      case CENT50:
      c50=0;
      	credit=credit+50;
      	printk("Credit: %d\n\r",credit);
      	state=IDLE;
      break;
      
      case CENT100:
      c100=0;
      	credit=credit+100;
      	printk("Credit: %d\n\r",credit);
      	state=IDLE;
      break;

      case RETURNING:
      c_return=0;
        printk("%d EUR credit return \n",credit);
        credit=0;
        state=IDLE;
      break;
      
      }/*switch(state)*/
      k_msleep(SLEEP_MS);
    }/*while(1)*/
  return;
}


/*Implementation of Dispesing superstate*/
void dispensing_superstate(){

  int16_t state1=COMPARISON;
  bool done=false;

  while(!done){
    switch(state1){
      case COMPARISON:
      	if(sel_prod==1 && credit>=beerCost) { state1=OK; }
      	else if(sel_prod==2 && credit>=tunaSandwichCost) { state1=OK; }
      	else if(sel_prod==3 && credit>=coffeeCost) { state1=OK; }
      	else state1=ERROR;
      break;
      
      case ERROR:
        if(sel_prod==1){
    	  printk("Not enough credit, product Beer cost 1.50 EUR, credit is %d\n",credit);
  	}
  	else if(sel_prod==2){
    	  printk("Not enough credit, product Tuna sandwich cost 1.00 EUR, credit is %d\n",credit);
  	}
  	else if(sel_prod==3){
    	  printk("Not enough credit, product Coffee cost 0.50 EUR, credit is %d\n",credit);
  	}
        state1=DISPENSE;
      break;
      
      case OK:
        if(sel_prod==1){
    	  credit=credit-150;
          printk("Product Beer dispensed, remaining credit %d\n",credit);
  	}
  	else if(sel_prod==2){
          credit=credit-100;
    	  printk("Product Tuna Sandwich dispensed, remaining credit %d\n",credit);
  	}
  	else if(sel_prod==3){
          credit=credit-50;
    	  printk("Product Coffee dispensed, remaining credit %d\n",credit);
  	}
        state1=DISPENSE;
      break;
    
