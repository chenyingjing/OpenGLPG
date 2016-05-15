//
//  main.cpp
//  HelloGLM
//
//  Created by aa64mac on 5/13/16.
//  Copyright © 2016 cyj. All rights reserved.
//

#include <iostream>
#include "glm/glm.hpp"

using namespace glm;

int main(int argc, const char * argv[]) {
    std::cout << "Hello, glm!\n";
    
    mat2 m(0, 1, 2, 3);
    
    mat2 result = m * m;
    
    if (result == mat2(2, 3, 6, 11)) {
        std::cout << "OK\n";
    } else {
        std::cout << "NG\n";
    }
    
    return 0;
}
