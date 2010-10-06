/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                             ClusteringSuite                               *
 *   Infrastructure and tools to apply clustering analysis to Paraver and    *
 *                              Dimemas traces                               *
 *                                                                           * 
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\

  $URL:: https://svn.bsc.#$:  File
  $Rev:: 20               $:  Revision of last commit
  $Author:: jgonzale      $:  Author of last commit
  $Date:: 2010-03-09 17:1#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef CPISTACKMODEL_H
#define CPISTACKMODEL_H

#define T_CYC 4.55e-10

#define CPISTACK_COUNTERS_NUM      49

#define PM_CYC                     0
#define PM_GRP_CMPL                1

#define PM_GCT_EMPTY_CYC           2
#define PM_GCT_EMPTY_IC_MISS       3
#define PM_GCT_EMPTY_BR_MPRED      4

#define PM_CMPLU_STALL_LSU         5
#define PM_CMPLU_STALL_REJECT      6
#define PM_CMPLU_STALL_ERAT_MISS   7
#define PM_CMPLU_STALL_DCACHE_MISS 8
#define PM_CMPLU_STALL_FXU         9
#define PM_CMPLU_STALL_DIV         10
#define PM_CMPLU_STALL_FPU         11
#define PM_CMPLU_STALL_FDIV        12

#define PM_CMPLU_STALL_OTHER       13

#define PM_INST_CMPL               14
#define PM_INST_DISP               15

#define PM_LD_REF_L1               16
#define PM_ST_REF_L1               17
#define PM_LD_MISS_L1              18
#define PM_ST_MISS_L1              19
#define PM_LD_MISS_L1_LSU0         20
#define PM_LD_MISS_L1_LSU1         21
#define PM_L1_WRITE_CYC            22
#define PM_DATA_FROM_MEM           23
#define PM_DATA_FROM_L2            24
#define PM_DTLB_MISS               25
#define PM_ITLB_MISS               26

#define PM_LSU0_BUSY               27
#define PM_LSU1_BUSY               28
#define PM_LSU_FLUSH               29
#define PM_LSU_LDF                 30

#define PM_FPU_FIN                 31
#define PM_FPU_FSQRT               32
#define PM_FPU_FDIV                33
#define PM_FPU_FMA                 34
#define PM_FPU0_FIN                35
#define PM_FPU1_FIN                36
#define PM_FPU_STF                 37

#define PM_LSU_LMQ_SRQ_EMPTY_CYC   38
#define PM_HV_CYC                  39
#define PM_1PLUS_PPC_CMPL          40
#define PM_TB_BIT_TRANS            41

#define PM_FLUSH_BR_MPRED          42
#define PM_BR_MPRED_TA             43

#define PM_GCT_EMPTY_SRQ_FULL      44
#define PM_FXU_FIN                 45
#define PM_FXU_BUSY                46

#define PM_IOPS_CMPL               47

#define PM_GCT_FULL_CYC            48


#define PM_CYC_NAME                     "PM_CYC"
#define PM_GRP_CMPL_NAME                "PM_GRP_CMPL"

#define PM_GCT_EMPTY_CYC_NAME           "PM_GCT_EMPTY_CYC"
#define PM_GCT_EMPTY_IC_MISS_NAME       "PM_GCT_EMPTY_IC_MISS"
#define PM_GCT_EMPTY_BR_MPRED_NAME      "PM_GCT_EMPTY_BR_MPRED"

#define PM_CMPLU_STALL_LSU_NAME         "PM_CMPLU_STALL_LSU"
#define PM_CMPLU_STALL_REJECT_NAME      "PM_CMPLU_STALL_REJECT"
#define PM_CMPLU_STALL_ERAT_MISS_NAME   "PM_CMPLU_STALL_ERAT_MISS"
#define PM_CMPLU_STALL_DCACHE_MISS_NAME "PM_CMPLU_STALL_DCACHE_MISS"
#define PM_CMPLU_STALL_FXU_NAME         "PM_CMPLU_STALL_FXU"
#define PM_CMPLU_STALL_DIV_NAME         "PM_CMPLU_STALL_DIV"
#define PM_CMPLU_STALL_FPU_NAME         "PM_CMPLU_STALL_FPU"
#define PM_CMPLU_STALL_FDIV_NAME        "PM_CMPLU_STALL_FDIV"

#define PM_CMPLU_STALL_OTHER_NAME       "PM_CMPLU_STALL_OTHER"

#define PM_INST_CMPL_NAME               "PM_INST_CMPL"
#define PM_INST_DISP_NAME               "PM_INST_DISP"

#define PM_LD_REF_L1_NAME               "PM_LD_REF_L1"
#define PM_ST_REF_L1_NAME               "PM_ST_REF_L1"
#define PM_LD_MISS_L1_NAME              "PM_LD_MISS_L1"
#define PM_ST_MISS_L1_NAME              "PM_ST_MISS_L1"
#define PM_LD_MISS_L1_LSU0_NAME         "PM_LD_MISS_L1_LSU0"
#define PM_LD_MISS_L1_LSU1_NAME         "PM_LD_MISS_L1_LSU1"
#define PM_L1_WRITE_CYC_NAME            "PM_L1_WRITE_CYC"
#define PM_DATA_FROM_MEM_NAME           "PM_DATA_FROM_MEM"
#define PM_DATA_FROM_L2_NAME            "PM_DATA_FROM_L2"
#define PM_DTLB_MISS_NAME               "PM_DTLB_MISS"
#define PM_ITLB_MISS_NAME               "PM_ITLB_MISS"

#define PM_LSU0_BUSY_NAME               "PM_LSU0_BUSY"
#define PM_LSU1_BUSY_NAME               "PM_LSU1_BUSY"
#define PM_LSU_FLUSH_NAME               "PM_LSU_FLUSH"
#define PM_LSU_LDF_NAME                 "PM_LSU_LDF"

#define PM_FPU_FIN_NAME                 "PM_FPU_FIN"
#define PM_FPU_FSQRT_NAME               "PM_FPU_FSQRT"
#define PM_FPU_FDIV_NAME                "PM_FPU_FDIV"
#define PM_FPU_FMA_NAME                 "PM_FPU_FMA"
#define PM_FPU0_FIN_NAME                "PM_FPU0_FIN"
#define PM_FPU1_FIN_NAME                "PM_FPU1_FIN"
#define PM_FPU_STF_NAME                 "PM_FPU_STF"
#define PM_FPU_FDIV_NAME                "PM_FPU_FDIV"
#define PM_FPU_FMA_NAME                 "PM_FPU_FMA"

#define PM_LSU_LMQ_SRQ_EMPTY_CYC_NAME   "PM_LSU_LMQ_SRQ_EMPTY_CYC"
#define PM_HV_CYC_NAME                  "PM_HV_CYC"
#define PM_1PLUS_PPC_CMPL_NAME          "PM_1PLUS_PPC_CMPL"
#define PM_TB_BIT_TRANS_NAME            "PM_TB_BIT_TRANS"

#define PM_FLUSH_BR_MPRED_NAME          "PM_FLUSH_BR"
#define PM_BR_MPRED_TA_NAME             "PM_BR_MPRED_TA"

#define PM_GCT_EMPTY_SRQ_FULL_NAME      "PM_GCT_EMPTY_SRQ_FULL"
#define PM_FXU_FIN_NAME                 "PM_FXU_FIN"
#define PM_FXU_BUSY_NAME                "PM_FXU_BUSY"

#define PM_IOPS_CMPL_NAME               "PM_IOPS_CMPL"

#define PM_GCT_FULL_CYC_NAME            "PM_GCT_FULL_CYC"

#define PM_CYC_VAL                     42001008
#define PM_GRP_CMPL_VAL                42001079

#define PM_GCT_EMPTY_CYC_VAL           42001073
#define PM_GCT_EMPTY_IC_MISS_VAL       42001228
#define PM_GCT_EMPTY_BR_MPRED_VAL      42001229

#define PM_CMPLU_STALL_LSU_VAL         42001218
#define PM_CMPLU_STALL_REJECT_VAL      42001222
#define PM_CMPLU_STALL_ERAT_MISS_VAL   42001219
#define PM_CMPLU_STALL_DCACHE_MISS_VAL 42001221
#define PM_CMPLU_STALL_FXU_VAL         42001223
#define PM_CMPLU_STALL_DIV_VAL         42001224
#define PM_CMPLU_STALL_FPU_VAL         42001227
#define PM_CMPLU_STALL_FDIV_VAL        42001226

#define PM_CMPLU_STALL_OTHER_VAL       42001220

#define PM_INST_CMPL_VAL               42001090
#define PM_INST_DISP_VAL               42001091

#define PM_LD_REF_L1_VAL               42001108
#define PM_ST_REF_L1_VAL               42001207
#define PM_LD_MISS_L1_VAL              42001105
#define PM_ST_MISS_L1_VAL              42001206
#define PM_LD_MISS_L1_LSU0_VAL         42001106
#define PM_LD_MISS_L1_LSU1_VAL         42001107
#define PM_L1_WRITE_CYC_VAL            42001102
#define PM_DATA_FROM_MEM_VAL           42001012
#define PM_DATA_FROM_L2_VAL            42001009
#define PM_DTLB_MISS_VAL               42001018
#define PM_ITLB_MISS_VAL               42001099

#define PM_LSU0_BUSY_VAL               42001118
#define PM_LSU1_BUSY_VAL               42001129
#define PM_LSU_FLUSH_VAL               42001135
#define PM_LSU_LDF_VAL                 42001140

#define PM_FPU_FIN_VAL                 42001056
#define PM_FPU_FSQRT_VAL               42001060
#define PM_FPU_FDIV_VAL                42001054
#define PM_FPU_FMA_VAL                 42001057
#define PM_FPU0_FIN_VAL                42001029
#define PM_FPU1_FIN_VAL                42001043
#define PM_FPU_STF_VAL                 42001063

#define PM_LSU_LMQ_SRQ_EMPTY_CYC_VAL   42001145
#define PM_HV_CYC_VAL                  42001086
#define PM_1PLUS_PPC_CMPL_VAL          42001001
#define PM_TB_BIT_TRANS_VAL            42001211

#define PM_FLUSH_BR_MPRED_VAL          42001022
#define PM_BR_MPRED_TA_VAL             42001005

#define PM_GCT_EMPTY_SRQ_FULL_VAL      42001074
#define PM_FXU_FIN_VAL                 42001071
#define PM_FXU_BUSY_VAL                42001070

#define PM_IOPS_CMPL_VAL               42001225

#define PM_GCT_FULL_CYC_VAL            42001075


#define HEADER_STR  "ClId,A, B, B1, B2, B3, C, C1, C1A, C1A1, C1A2, C1B, C1C, C2, C2A, C2B, C3, C3A, C3B, C4"
#define HWC_HEADER_STR "ClId, PM_CYC, PM_GRP_CMPL, PM_GCT_EMPTY_CYC, PM_GCT_EMPTY_IC_MISS, PM_GCT_EMPTY_BR_MPRED, PM_CMPLU_STALL_LSU, PM_CMPLU_STALL_REJECT, PM_CMPLU_STALL_ERAT_MISS, PM_CMPLU_STALL_DCACHE_MISS, PM_CMPLU_STALL_FXU, PM_CMPLU_STALL_DIV, PM_CMPLU_STALL_FPU, PM_CMPLU_STALL_FDIV, PM_INST_CMPL, PM_INST_DISP, PM_LD_MISS_L1, PM_DATA_FROM_MEM, PM_FPU_FIN"

#define CLID_TXT      "  ClusterID                           "
#define BASIC_TXT     "* Basic metrics                       "
#define SEPARATOR     "  |"
#define TOTAL_DUR_TXT "  |--> Total duration                 "
#define REL_DUR_TXT   "  |--> Total duration (%)             "
#define IPC_TXT       "  |--> IPC                            "
#define MIPS_TXT      "  |--> MIPS                           "
#define MFLOPS_TXT    "  |--> MFLOPS                         "
#define L1M_TXT       "  |--> L1 Data Misses/KInstr          "
#define L2M_TXT       "  |--> L2 Data Misses/KInstr          "
#define MEMBW_TXT     "  |--> Memory BW                      "

#define CPISTACK_TXT  "* CPIStack Model                      "
#define SEPARATOR     "  |"
#define A_TXT         "  |--> Completition Cycles            "
#define SEPARATOR     "  |"
#define B_TXT         "  |--> Completition Table Empty       "
#define B1_TXT        "  |    |--> I-Cache Miss Penalty      "
#define B2_TXT        "  |    |--> Branch redirection        "
#define B3_TXT        "  |    |--> Others                    "
#define C_TXT         "  |--> Completion Stall Cycles        "
#define C1_TXT        "       |--> Stall by LSU instruction  "
#define C1A_TXT       "       |    |--> Stall by reject      "
#define C1A1_TXT      "       |    |    |--> Translation     "
#define C1A2_TXT      "       |    |    |--> Other reject    "
#define C1B_TXT       "       |    |--> D-Cache miss         "
#define C1C_TXT       "       |    |--> LSU Basic Latency    "
#define SEPARATOR2    "       | "
#define C2_TXT        "       |--> Stall by FXU instruction  "
#define C2A_TXT       "       |    |--> DIV/MTSPR/ecc        "
#define C2C_TXT       "       |    |--> FXU basic latency    "
#define C3_TXT        "       |--> Stall by FPU instruction  "
#define C3A_TXT       "       |    |--> FDIV/FSQRT/ecc       "
#define C3B_TXT       "       |    |--> FPU basic latency    "
#define C4_TXT        "       |--> Stall by others           "

#define CLID_CSV      "ClusterID"
#define TOTAL_DUR_CSV "Total duration"
#define REL_DUR_CSV   "Total duration (%)"
#define IPC_CSV       "IPC"
#define MIPS_CSV      "MIPS"
#define MFLOPS_CSV    "MFLOPS"
#define L1M_CSV       "L1 Data Misses/KInstr"
#define L2M_CSV       "L2 Data Misses/KInstr"
#define MEMBW_CSV     "Memory BW"

#define A_CSV         "A: Completition Cycles"
#define B_CSV         "B: Completition Table Empty"
#define B1_CSV        "B1: I-Cache Miss Penalty"
#define B2_CSV        "B2: Branch redirection"
#define B3_CSV        "B3: Others"
#define C_CSV         "C: Completion Stall Cycles"
#define C1_CSV        "C1: Stall by LSU instruction"
#define C1A_CSV       "C1A: Stall by reject"
#define C1A1_CSV      "C1A1: Translation"
#define C1A2_CSV      "C1A2: Other reject"
#define C1B_CSV       "C1B: D-Cache miss"
#define C1C_CSV       "C1C: LSU Basic Latency"
#define C2_CSV        "C2: Stall by FXU instruction"
#define C2A_CSV       "C2A: DIV/MTSPR/ecc"
#define C2C_CSV       "C2C: FXU basic latency"
#define C3_CSV        "C3: Stall by FPU instruction"
#define C3A_CSV       "C3A: FDIV/FSQRT/ecc"
#define C3B_CSV       "C3B: FPU basic latency"
#define C4_CSV        "C4: Stall by others"

#endif /* CPISTACKMODEL_H */
