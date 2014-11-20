
/*
  WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

  This file was generated from HelloWorld.idl using "rtiddsgen".
  The rtiddsgen tool is part of the RTI Connext distribution.
  For more information, type 'rtiddsgen -help' at a command shell
  or consult the RTI Connext manual.
*/

#ifndef HelloWorldPlugin_1436886087_h
#define HelloWorldPlugin_1436886087_h

#include "HelloWorld.h"




struct RTICdrStream;

#ifndef pres_typePlugin_h
#include "pres/pres_typePlugin.h"
#endif


#if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, start exporting symbols.
*/
#undef NDDSUSERDllExport
#define NDDSUSERDllExport __declspec(dllexport)
#endif


#ifdef __cplusplus
extern "C" {
#endif

#define SampleType_LAST_MEMBER_ID 0

#define SampleTypePlugin_get_sample PRESTypePluginDefaultEndpointData_getSample 
#define SampleTypePlugin_return_sample PRESTypePluginDefaultEndpointData_returnSample 
#define SampleTypePlugin_get_buffer PRESTypePluginDefaultEndpointData_getBuffer 
#define SampleTypePlugin_return_buffer PRESTypePluginDefaultEndpointData_returnBuffer 
 

#define SampleTypePlugin_create_sample PRESTypePluginDefaultEndpointData_createSample 
#define SampleTypePlugin_destroy_sample PRESTypePluginDefaultEndpointData_deleteSample 

/* --------------------------------------------------------------------------------------
    Support functions:
 * -------------------------------------------------------------------------------------- */

NDDSUSERDllExport extern SampleType*
SampleTypePluginSupport_create_data_ex(RTIBool allocate_pointers);

NDDSUSERDllExport extern SampleType*
SampleTypePluginSupport_create_data(void);

NDDSUSERDllExport extern RTIBool 
SampleTypePluginSupport_copy_data(
    SampleType *out,
    const SampleType *in);

NDDSUSERDllExport extern void 
SampleTypePluginSupport_destroy_data_ex(
    SampleType *sample,RTIBool deallocate_pointers);

NDDSUSERDllExport extern void 
SampleTypePluginSupport_destroy_data(
    SampleType *sample);

NDDSUSERDllExport extern void 
SampleTypePluginSupport_print_data(
    const SampleType *sample,
    const char *desc,
    unsigned int indent);


/* ----------------------------------------------------------------------------
    Callback functions:
 * ---------------------------------------------------------------------------- */

NDDSUSERDllExport extern PRESTypePluginRTPSParticipantData 
SampleTypePlugin_on_RTPSParticipant_attached(
    void *registration_data, 
    const struct PRESTypePluginRTPSParticipantInfo *RTPSParticipant_info,
    RTIBool top_level_registration, 
    void *container_plugin_context,
    RTICdrTypeCode *typeCode);

NDDSUSERDllExport extern void 
SampleTypePlugin_on_RTPSParticipant_detached(
    PRESTypePluginRTPSParticipantData RTPSParticipant_data);
    
NDDSUSERDllExport extern PRESTypePluginEndpointData 
SampleTypePlugin_on_endpoint_attached(
    PRESTypePluginRTPSParticipantData RTPSParticipant_data,
    const struct PRESTypePluginEndpointInfo *endpoint_info,
    RTIBool top_level_registration, 
    void *container_plugin_context);

NDDSUSERDllExport extern void 
SampleTypePlugin_on_endpoint_detached(
    PRESTypePluginEndpointData endpoint_data);


NDDSUSERDllExport extern RTIBool 
SampleTypePlugin_copy_sample(
    PRESTypePluginEndpointData endpoint_data,
    SampleType *out,
    const SampleType *in);

/* --------------------------------------------------------------------------------------
    (De)Serialize functions:
 * -------------------------------------------------------------------------------------- */

NDDSUSERDllExport extern RTIBool 
SampleTypePlugin_serialize(
    PRESTypePluginEndpointData endpoint_data,
    const SampleType *sample,
    struct RTICdrStream *stream, 
    RTIBool serialize_encapsulation,
    RTIEncapsulationId encapsulation_id,
    RTIBool serialize_sample, 
    void *endpoint_plugin_qos);

NDDSUSERDllExport extern RTIBool 
SampleTypePlugin_deserialize_sample(
    PRESTypePluginEndpointData endpoint_data,
    SampleType *sample, 
    struct RTICdrStream *stream,
    RTIBool deserialize_encapsulation,
    RTIBool deserialize_sample, 
    void *endpoint_plugin_qos);

 
NDDSUSERDllExport extern RTIBool 
SampleTypePlugin_deserialize(
    PRESTypePluginEndpointData endpoint_data,
    SampleType **sample, 
    RTIBool * drop_sample,
    struct RTICdrStream *stream,
    RTIBool deserialize_encapsulation,
    RTIBool deserialize_sample, 
    void *endpoint_plugin_qos);




NDDSUSERDllExport extern RTIBool
SampleTypePlugin_skip(
    PRESTypePluginEndpointData endpoint_data,
    struct RTICdrStream *stream, 
    RTIBool skip_encapsulation,  
    RTIBool skip_sample, 
    void *endpoint_plugin_qos);

NDDSUSERDllExport extern unsigned int 
SampleTypePlugin_get_serialized_sample_max_size(
    PRESTypePluginEndpointData endpoint_data,
    RTIBool include_encapsulation,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment);

NDDSUSERDllExport extern unsigned int 
SampleTypePlugin_get_serialized_sample_min_size(
    PRESTypePluginEndpointData endpoint_data,
    RTIBool include_encapsulation,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment);

NDDSUSERDllExport extern unsigned int
SampleTypePlugin_get_serialized_sample_size(
    PRESTypePluginEndpointData endpoint_data,
    RTIBool include_encapsulation,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment,
    const SampleType * sample);



/* --------------------------------------------------------------------------------------
    Key Management functions:
 * -------------------------------------------------------------------------------------- */

NDDSUSERDllExport extern PRESTypePluginKeyKind 
SampleTypePlugin_get_key_kind(void);

NDDSUSERDllExport extern unsigned int 
SampleTypePlugin_get_serialized_key_max_size(
    PRESTypePluginEndpointData endpoint_data,
    RTIBool include_encapsulation,
    RTIEncapsulationId encapsulation_id,
    unsigned int current_alignment);

NDDSUSERDllExport extern RTIBool 
SampleTypePlugin_serialize_key(
    PRESTypePluginEndpointData endpoint_data,
    const SampleType *sample,
    struct RTICdrStream *stream,
    RTIBool serialize_encapsulation,
    RTIEncapsulationId encapsulation_id,
    RTIBool serialize_key,
    void *endpoint_plugin_qos);

NDDSUSERDllExport extern RTIBool 
SampleTypePlugin_deserialize_key_sample(
    PRESTypePluginEndpointData endpoint_data,
    SampleType * sample,
    struct RTICdrStream *stream,
    RTIBool deserialize_encapsulation,
    RTIBool deserialize_key,
    void *endpoint_plugin_qos);

 
NDDSUSERDllExport extern RTIBool 
SampleTypePlugin_deserialize_key(
    PRESTypePluginEndpointData endpoint_data,
    SampleType ** sample,
    RTIBool * drop_sample,
    struct RTICdrStream *stream,
    RTIBool deserialize_encapsulation,
    RTIBool deserialize_key,
    void *endpoint_plugin_qos);


NDDSUSERDllExport extern RTIBool
SampleTypePlugin_serialized_sample_to_key(
    PRESTypePluginEndpointData endpoint_data,
    SampleType *sample,
    struct RTICdrStream *stream, 
    RTIBool deserialize_encapsulation,  
    RTIBool deserialize_key, 
    void *endpoint_plugin_qos);

     
/* Plugin Functions */
NDDSUSERDllExport extern struct PRESTypePlugin*
SampleTypePlugin_new(void);

NDDSUSERDllExport extern void
SampleTypePlugin_delete(struct PRESTypePlugin *);

#ifdef __cplusplus
}
#endif

        
#if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, stop exporting symbols.
*/
#undef NDDSUSERDllExport
#define NDDSUSERDllExport
#endif        

#endif /* HelloWorldPlugin_1436886087_h */
