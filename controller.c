#include "controller.h"

void *inputHandler(){

    // Listen to user input for commands
    while(1){

        char input = getchar();

        switch(input){
            case (' '):
                controls.pause = !controls.pause;
                sleep(1);
                break;

            case ('f'):
                controls.fast_forward = !controls.fast_forward;
                sleep(1);
                break;
        }
    }

}