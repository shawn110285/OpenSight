##-----------------------------------------------------------------------------
## File:    makefile
## Author:  shawn Liu
## E-mail:  shawn110285@gmail.com
##-----------------------------------------------------------------------------

## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
##-----------------------------------------------------------------------------*/

export CC = gcc
export AR = ar
export AR_CFG = rcs

export CFLAGS = -g -fPIE -rdynamic

export CURR_DIR = $(shell pwd)
export OBJ_DIR = $(CURR_DIR)/build

PROG_NAME = OpenSight
BUILD_DIR = osal telnet_svr gdb_svr coresight

LIBS:=  -L$(OBJ_DIR) -Xlinker "-(" -losal -ltelnet -lgdb -ldap -lm -lrt -lpthread -lyaml -lusb-1.0 -ldl -Xlinker "-)"



.PHONY: all clean

all:
	@for subdir in $(BUILD_DIR);do cd $$subdir && make all && cd $(CURR_DIR) && echo "";done
	@echo "Finish"
	@echo ""

	@echo "Linking $(PROG_NAME)"
	$(CC) $(CFLAG) $(LIBS) -o $(OBJ_DIR)/$(PROG_NAME) $(LIBS)
	@echo "Finish"
	@echo ""

clean:
	@echo "cleaning target"
	@for subdir in $(BUILD_DIR);do cd $$subdir && make clean && cd $(CURR_DIR) && echo "";done
	@rm -rf $(OBJ_DIR)/*.a
	@rm -rf $(OBJ_DIR)/$(PROG_NAME)
	@echo "clean finish"