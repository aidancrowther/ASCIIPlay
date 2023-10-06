#include "controller.h"

void *inputHandler(){

    // Listen to user input for commands
    while(!controls.cease_execution){

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

            case (27):
                controls.cease_execution = true;
                break;
        }
    }

}