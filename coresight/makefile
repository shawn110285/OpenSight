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

# dp-jtag

# dp
MODULE_DAP_DP_DIR:=./dap/dp/
MODULE_DAP_DP_TMP := $(wildcard $(MODULE_DAP_DP_DIR)/*.c)
MODULE_DAP_DP_SRC :=$(notdir $(MODULE_DAP_DP_TMP))
MODULE_DAP_DP_OBJS:=$(MODULE_DAP_DP_SRC:.c=)

# jtag-ap
MODULE_DAP_JTAGAP_DIR:=./dap/jtag_ap/
MODULE_DAP_JTAGAP_TMP := $(wildcard $(MODULE_DAP_JTAGAP_DIR)/*.c)
MODULE_DAP_JTAGAP_SRC :=$(notdir $(MODULE_DAP_JTAGAP_TMP))
MODULE_DAP_JTAGAP_OBJS:=$(MODULE_DAP_JTAGAP_SRC:.c=)

# apb-ap
MODULE_DAP_APBAP_DIR:=./dap/apb_ap/
MODULE_DAP_APBAP_TMP := $(wildcard $(MODULE_DAP_APBAP_DIR)/*.c)
MODULE_DAP_APBAP_SRC :=$(notdir $(MODULE_DAP_APBAP_TMP))
MODULE_DAP_APBAP_OBJS:=$(MODULE_DAP_APBAP_SRC:.c=)

# funnel
MODULE_FUNNEL_DIR:=./funnel/
MODULE_FUNNEL_TMP := $(wildcard $(MODULE_FUNNEL_DIR)/*.c)
MODULE_FUNNEL_SRC :=$(notdir $(MODULE_FUNNEL_TMP))
MODULE_FUNNEL_OBJS:=$(MODULE_FUNNEL_SRC:.c=)

# replicator
MODULE_REPLICATOR_DIR:=./replicator/
MODULE_REPLICATOR_TMP := $(wildcard $(MODULE_REPLICATOR_DIR)/*.c)
MODULE_REPLICATOR_SRC :=$(notdir $(MODULE_REPLICATOR_TMP))
MODULE_REPLICATOR_OBJS:=$(MODULE_REPLICATOR_SRC:.c=)

# etb
MODULE_ETB_DIR:=./etb/
MODULE_ETB_TMP := $(wildcard $(MODULE_ETB_DIR)/*.c)
MODULE_ETB_SRC :=$(notdir $(MODULE_ETB_TMP))
MODULE_ETB_OBJS:=$(MODULE_ETB_SRC:.c=)

# tpiu
MODULE_TPIU_DIR:=./tpiu/
MODULE_TPIU_TMP := $(wildcard $(MODULE_TPIU_DIR)/*.c)
MODULE_TPIU_SRC :=$(notdir $(MODULE_TPIU_TMP))
MODULE_TPIU_OBJS:=$(MODULE_TPIU_SRC:.c=)

# entry
MODULE_ENTRY_DIR:=./
MODULE_ENTRY_TMP := $(wildcard $(MODULE_ENTRY_DIR)/*.c)
MODULE_ENTRY_SRC :=$(notdir $(MODULE_ENTRY_TMP))
MODULE_ENTRY_OBJS:=$(MODULE_ENTRY_SRC:.c=)

MODULE_OBJS:=$(MODULE_DAP_DP_SRC:.c=.o)
MODULE_OBJS+=$(MODULE_DAP_JTAGAP_SRC:.c=.o)
MODULE_OBJS+=$(MODULE_DAP_APBAP_SRC:.c=.o)
MODULE_OBJS+=$(MODULE_FUNNEL_SRC:.c=.o)
MODULE_OBJS+=$(MODULE_REPLICATOR_SRC:.c=.o)
MODULE_OBJS+=$(MODULE_ETB_SRC:.c=.o)
MODULE_OBJS+=$(MODULE_TPIU_SRC:.c=.o)
MODULE_OBJS+=$(MODULE_ENTRY_SRC:.c=.o)



MODULE_LIB:=libdap.a


all:
#   dp
	echo "Compile coresight dp module"
	for obj in $(MODULE_DAP_DP_OBJS);do \
	echo "Building $$obj.c --> $$obj.o" && \
	$(CC) -c $(CFLAGS) $(DEBUG_DEFINE) -o $$obj.o $(MODULE_DAP_DP_DIR)$$obj.c  &&\
	echo "Finish"&&\
	echo "";\
	done

#   JTAG-AP
	echo "Compile coresight jtag-ap module"
	for obj in $(MODULE_DAP_JTAGAP_OBJS);do \
	echo "Building $$obj.c --> $$obj.o" && \
	$(CC) -c $(CFLAGS) $(DEBUG_DEFINE) -o $$obj.o $(MODULE_DAP_JTAGAP_DIR)$$obj.c  &&\
	echo "Finish"&&\
	echo "";\
	done

# APB-AP
	echo "Compile coresight apb-ap module"
	for obj in $(MODULE_DAP_APBAP_OBJS);do \
	echo "Building $$obj.c --> $$obj.o" && \
	$(CC) -c $(CFLAGS) $(DEBUG_DEFINE) -o $$obj.o $(MODULE_DAP_APBAP_DIR)$$obj.c  &&\
	echo "Finish"&&\
	echo "";\
	done

# funnel
	echo "Compile coresight funnel module"
	for obj in $(MODULE_FUNNEL_OBJS);do \
	echo "Building $$obj.c --> $$obj.o" && \
	$(CC) -c $(CFLAGS) $(DEBUG_DEFINE) -o $$obj.o $(MODULE_FUNNEL_DIR)$$obj.c  &&\
	echo "Finish"&&\
	echo "";\
	done

# replicator
	echo "Compile coresight replicator module"
	for obj in $(MODULE_REPLICATOR_OBJS);do \
	echo "Building $$obj.c --> $$obj.o" && \
	$(CC) -c $(CFLAGS) $(DEBUG_DEFINE) -o $$obj.o $(MODULE_REPLICATOR_DIR)$$obj.c  &&\
	echo "Finish"&&\
	echo "";\
	done

# ETB
	echo "Compile coresight etb module"
	for obj in $(MODULE_ETB_OBJS);do \
	echo "Building $$obj.c --> $$obj.o" && \
	$(CC) -c $(CFLAGS) $(DEBUG_DEFINE) -o $$obj.o $(MODULE_ETB_DIR)$$obj.c  &&\
	echo "Finish"&&\
	echo "";\
	done

# tpiu
	echo "Compile coresight tpiu module"
	for obj in $(MODULE_TPIU_OBJS);do \
	echo "Building $$obj.c --> $$obj.o" && \
	$(CC) -c $(CFLAGS) $(DEBUG_DEFINE) -o $$obj.o $(MODULE_TPIU_DIR)$$obj.c  &&\
	echo "Finish"&&\
	echo "";\
	done

# entry
	echo "Compile coresight entry"
	for obj in $(MODULE_ENTRY_OBJS);do \
	echo "Building $$obj.c --> $$obj.o" && \
	$(CC) -c $(CFLAGS) $(DEBUG_DEFINE) -o $$obj.o $(MODULE_ENTRY_DIR)$$obj.c  &&\
	echo "Finish"&&\
	echo "";\
	done

# archive the obj files into a lib
	@echo "Archiving $(MODULE_OBJS) To $(MODULE_LIB)"
	$(AR) $(AR_CFG) $(MODULE_LIB) $(MODULE_OBJS)
	@echo "Finish"
	@echo ""
# cp lib to the build dir
	@echo "cp $(MODULE_LIB) ---> $(OBJ_DIR)/$(MODULE_LIB)"
	@cp $(MODULE_LIB) $(OBJ_DIR)/$(MODULE_LIB)
	@echo ""

clean:
	@echo "cleaning objs and $(MODULE_LIB)"
	@rm -rf $(MODULE_OBJS) $(MODULE_LIB)
	@echo "Finish"
