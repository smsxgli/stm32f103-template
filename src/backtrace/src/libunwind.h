#ifndef LIBUNWIND_H_DEFINED
#define LIBUNWIND_H_DEFINED

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
  _URC_OK = 0,
  /* operation completed successfully */
  _URC_FOREIGN_EXCEPTION_CAUGHT = 1,
  _URC_HANDLER_FOUND = 6,
  _URC_INSTALL_CONTEXT = 7,
  _URC_CONTINUE_UNWIND = 8,
  _URC_FAILURE = 9 /* unspecified failure of some kind */
} _Unwind_Reason_Code;

typedef uint32_t _Unwind_State;

typedef struct _Unwind_Control_Block _Unwind_Control_Block;
typedef struct _Unwind_Context _Unwind_Context;
typedef uint32_t _Unwind_EHT_Header;

typedef struct _Unwind_Control_Block {
  char exception_class[8];
  void (*exception_cleanup)(_Unwind_Reason_Code, _Unwind_Control_Block *);
  /* Unwinder cache, private fields for the unwinder's use */
  struct {
    uint32_t reserved1;
    /* init reserved1 to 0, then don't touch */
    uint32_t reserved2;
    uint32_t reserved3;
    uint32_t reserved4;
    uint32_t reserved5;
  } unwinder_cache;
  /* Propagation barrier cache (valid after phase 1): */
  struct {
    uint32_t sp;
    uint32_t bitpattern[5];
  } barrier_cache;
  /* Cleanup cache (preserved over cleanup): */
  struct {
    uint32_t bitpattern[4];
  } cleanup_cache;
  /* Pr cache (for pr's benefit): */
  struct {
    uint32_t fnstart;
    /* function start address */
    _Unwind_EHT_Header *ehtp;
    /* pointer to EHT entry header word */
    uint32_t additional;
    /* additional data */
    uint32_t reserved1;
  } pr_cache;
  long long int : 0;
  /* Force alignment of next item to 8-byte boundary */
} _Unwind_Control_Block;

/* Unwinding functions */
extern _Unwind_Reason_Code _Unwind_RaiseException(_Unwind_Control_Block *ucbp);
extern void _Unwind_Resume(_Unwind_Control_Block *ucbp);
extern void _Unwind_Complete(_Unwind_Control_Block *ucbp);
extern void _Unwind_DeleteException(_Unwind_Control_Block *ucbp);

typedef enum {
  _UVRSC_CORE = 0,
  _UVRSC_VFP = 1,
  _UVRSC_WMMXD = 3,
  _UVRSC_WMMXC = 4,
  _UVRSC_PSEUDO = 5
} _Unwind_VRS_RegClass;
/* integer register */
/* vfp */
/* Intel WMMX data register */
/* Intel WMMX control register */
/* Special purpose pseudo register */
typedef enum {
  _UVRSD_UINT32 = 0,
  _UVRSD_VFPX = 1,
  _UVRSD_UINT64 = 3,
  _UVRSD_FLOAT = 4,
  _UVRSD_DOUBLE = 5
} _Unwind_VRS_DataRepresentation;
typedef enum {
  _UVRSR_OK = 0,
  _UVRSR_NOT_IMPLEMENTED = 1,
  _UVRSR_FAILED = 2
} _Unwind_VRS_Result;

extern _Unwind_VRS_Result
_Unwind_VRS_Set(_Unwind_Context *context, _Unwind_VRS_RegClass regclass,
                uint32_t regno, _Unwind_VRS_DataRepresentation representation,
                void *valuep);

extern _Unwind_VRS_Result
_Unwind_VRS_Get(_Unwind_Context *context, _Unwind_VRS_RegClass regclass,
                uint32_t regno, _Unwind_VRS_DataRepresentation representation,
                void *valuep);

extern _Unwind_VRS_Result
_Unwind_VRS_Pop(_Unwind_Context *context, _Unwind_VRS_RegClass regclass,
                uint32_t discriminator,
                _Unwind_VRS_DataRepresentation representation);

extern _Unwind_Reason_Code __aeabi_unwind_cpp_pr0(_Unwind_State state,
                                                  _Unwind_Control_Block *ucbp,
                                                  _Unwind_Context *context);
extern _Unwind_Reason_Code __aeabi_unwind_cpp_pr1(_Unwind_State state,
                                                  _Unwind_Control_Block *ucbp,
                                                  _Unwind_Context *context);
extern _Unwind_Reason_Code __aeabi_unwind_cpp_pr2(_Unwind_State state,
                                                  _Unwind_Control_Block *ucbp,
                                                  _Unwind_Context *context);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* LIBUNWIND_H_DEFINED */
