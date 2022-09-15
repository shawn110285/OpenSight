##-----------------------------------------------------------------------------
## File:    makefile
## Author:  Shawn Liu
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

MODULE_NAME := telnet

MODULE_LIB:=lib$(MODULE_NAME).a
MODULE_DIR:=./source/
MODULE_SRC_TMP := $(wildcard $(MODULE_DIR)/*.c)
MODULE_SRC :=$(notdir $(MODULE_SRC_TMP))
MODULE_OBJS:=$(MODULE_SRC:.c=.o)


%.o:$(MODULE_DIR)%.c
	@echo "Building $< --> $@"
	$(CC) -c $(CFLAGS) $(DEBUG_DEFINE) -o $@ $<
	@echo "Finish"
	@echo ""


all:$(MODULE_OBJS)
	@echo "Archiving $(MODULE_OBJS) To $(MODULE_LIB)"
	$(AR) $(AR_CFG) $(MODULE_LIB) $(MODULE_OBJS)
	@echo "Finish"
	@echo ""

	@echo "cp $(MODULE_LIB) ---> $(OBJ_DIR)/$(MODULE_LIB)"
	cp $(MODULE_LIB) $(OBJ_DIR)/$(MODULE_LIB)
	@echo ""

clean:
	@echo "cleaning objs and $(MODULE_LIB)"
	@rm -rf $(MODULE_OBJS) $(MODULE_LIB)
	@rm -rf $(OBJ_DIR)/$(MODULE_LIB)
	@echo "Finish"
