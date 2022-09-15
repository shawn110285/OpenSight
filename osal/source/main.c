//-----------------------------------------------------------------------------
// File:    main.c
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
//-----------------------------------------------------------------------------

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//-----------------------------------------------------------------------------

#include "../include/osalapi.h"

extern  void       osal_set_product_name(char * strProductName);
extern  void       osal_set_root_dir(char * pcRootDir);
extern  int32_t    osal_init_system(void);
extern  int32_t    osal_start_system(void);
extern  uint32_t   osal_task_delay(uint32_t  dwMillSecond);

int main(void)
{
    osal_set_product_name("OpenSight");
	osal_set_root_dir("./");

    if (OSAL_FALSE != osal_init_system())
    {
        osal_start_system();
    }
    else
    {
        printf("main: call osal_init_system Failed! \n");
        return EXIT_FAILURE;
    }

    while (1)
    {
        osal_task_delay(2000);
    }
    return EXIT_SUCCESS;
}

