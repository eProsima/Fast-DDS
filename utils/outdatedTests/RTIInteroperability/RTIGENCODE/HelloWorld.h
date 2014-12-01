
/*
  WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

  This file was generated from HelloWorld.idl using "rtiddsgen".
  The rtiddsgen tool is part of the RTI Connext distribution.
  For more information, type 'rtiddsgen -help' at a command shell
  or consult the RTI Connext manual.
*/

#ifndef HelloWorld_1436886087_h
#define HelloWorld_1436886087_h

#ifndef NDDS_STANDALONE_TYPE
    #ifdef __cplusplus
        #ifndef ndds_cpp_h
            #include "ndds/ndds_cpp.h"
        #endif
    #else
        #ifndef ndds_c_h
            #include "ndds/ndds_c.h"
        #endif
    #endif
#else
    #include "ndds_standalone_type.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

        
extern const char *SampleTypeTYPENAME;
        

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
    struct SampleTypeSeq;

#ifndef NDDS_STANDALONE_TYPE
    class SampleTypeTypeSupport;
    class SampleTypeDataWriter;
    class SampleTypeDataReader;
#endif

#endif

            
    
class SampleType                                        
{
public:            
#ifdef __cplusplus
    typedef struct SampleTypeSeq Seq;

#ifndef NDDS_STANDALONE_TYPE
    typedef SampleTypeTypeSupport TypeSupport;
    typedef SampleTypeDataWriter DataWriter;
    typedef SampleTypeDataReader DataReader;
#endif

#endif
    
    DDS_Long  sampleId;

            
};                        
    
                            
#if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
  /* If the code is building on Windows, start exporting symbols.
   */
  #undef NDDSUSERDllExport
  #define NDDSUSERDllExport __declspec(dllexport)
#endif

    
NDDSUSERDllExport DDS_TypeCode* SampleType_get_typecode(void); /* Type code */
    

DDS_SEQUENCE(SampleTypeSeq, SampleType);
        
NDDSUSERDllExport
RTIBool SampleType_initialize(
        SampleType* self);
        
NDDSUSERDllExport
RTIBool SampleType_initialize_ex(
        SampleType* self,RTIBool allocatePointers,RTIBool allocateMemory);

NDDSUSERDllExport
void SampleType_finalize(
        SampleType* self);
                        
NDDSUSERDllExport
void SampleType_finalize_ex(
        SampleType* self,RTIBool deletePointers);
        
NDDSUSERDllExport
RTIBool SampleType_copy(
        SampleType* dst,
        const SampleType* src);

#if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
  /* If the code is building on Windows, stop exporting symbols.
   */
  #undef NDDSUSERDllExport
  #define NDDSUSERDllExport
#endif



#endif /* HelloWorld_1436886087_h */
