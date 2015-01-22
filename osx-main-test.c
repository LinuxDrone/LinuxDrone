//
//  main.c
//  LinuxDrone
//
//  Created by Alexander Vrubel on 21.01.15.
//  Copyright (c) 2015 LinuxDrone. All rights reserved.
//

#include <stdio.h>
#include <apr_general.h>

int main(int argc, const char * argv[]) {
    
    apr_initialize();
    
    // insert code here...
    printf("Hello, World!\n");
    
    
    apr_terminate();

	getchar();
    return 0;
}
