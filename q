[1mdiff --git a/include/fastrtps/rtps/builtin/discovery/participant/PDP.h b/include/fastrtps/rtps/builtin/discovery/participant/PDP.h[m
[1mindex d440288f8..b59ad234f 100644[m
[1m--- a/include/fastrtps/rtps/builtin/discovery/participant/PDP.h[m
[1m+++ b/include/fastrtps/rtps/builtin/discovery/participant/PDP.h[m
[36m@@ -57,7 +57,7 @@[m [mclass RTPS_DllAPI PDP  // TODO: remove RTPS_DllAPI when discovery server incorpo[m
 {[m
     friend class RemoteRTPSParticipantLeaseDuration;[m
     friend class PDPListener;[m
[31m-    friend class PDPServerListener;[m
[32m+[m[32m    friend class BuiltinProtocols;[m
 [m
     public:[m
     /**[m
[36m@@ -67,6 +67,8 @@[m [mclass RTPS_DllAPI PDP  // TODO: remove RTPS_DllAPI when discovery server incorpo[m
     PDP(BuiltinProtocols* builtin);[m
     virtual ~PDP();[m
 [m
[32m+[m[32m    protected:[m
[32m+[m
     virtual void initializeParticipantProxyData(ParticipantProxyData* participant_data);[m
 [m
     /**[m
[36m@@ -105,6 +107,8 @@[m [mclass RTPS_DllAPI PDP  // TODO: remove RTPS_DllAPI when discovery server incorpo[m
      */[m
     bool addWriterProxyData(WriterProxyData* wdata, ParticipantProxyData &pdata);[m
 [m
[32m+[m[32m    public:[m
[32m+[m
     /**[m
      * This method returns a pointer to a ReaderProxyData object if it is found among the registered RTPSParticipants (including the local RTPSParticipant).[m
      * @param[in] reader GUID_t of the reader we are looking for.[m
[36m@@ -128,6 +132,9 @@[m [mclass RTPS_DllAPI PDP  // TODO: remove RTPS_DllAPI when discovery server incorpo[m
      * @return True if found.[m
      */[m
     bool lookupParticipantProxyData(const GUID_t& pguid, ParticipantProxyData& pdata);[m
[32m+[m
[32m+[m[32m    protected:[m
[32m+[m
     /**[m
      * This method removes and deletes a ReaderProxyData object from its corresponding RTPSParticipant.[m
      * @return true if found and deleted.[m
[36m@@ -168,6 +175,9 @@[m [mclass RTPS_DllAPI PDP  // TODO: remove RTPS_DllAPI when discovery server incorpo[m
 [m
     //!Pointer to the builtin protocols object.[m
     BuiltinProtocols* mp_builtin;[m
[32m+[m
[32m+[m[32m    public:[m
[32m+[m
     /**[m
      * Get a pointer to the local RTPSParticipant RTPSParticipantProxyData object.[m
      * @return Pointer to the local RTPSParticipant RTPSParticipantProxyData object.[m
[36m@@ -194,6 +204,8 @@[m [mclass RTPS_DllAPI PDP  // TODO: remove RTPS_DllAPI when discovery server incorpo[m
      */[m
     inline std::vector<ParticipantProxyData*>::const_iterator ParticipantProxiesEnd(){return m_participantProxies.end();};[m
 [m
[32m+[m[32m    protected:[m
[32m+[m
     /**[m
      * Assert the liveliness of a Remote Participant.[m
      * @param guidP GuidPrefix_t of the participant whose liveliness is being asserted.[m
[36m@@ -213,6 +225,8 @@[m [mclass RTPS_DllAPI PDP  // TODO: remove RTPS_DllAPI when discovery server incorpo[m
      */[m
     void assertRemoteWritersLiveliness(GuidPrefix_t& guidP,LivelinessQosPolicyKind kind);[m
 [m
[32m+[m[32m    public:[m
[32m+[m
     /**[m
      * Get the RTPS participant[m
      * @return RTPS participant[m
[1mdiff --git a/include/fastrtps/rtps/builtin/discovery/participant/PDPClient.h b/include/fastrtps/rtps/builtin/discovery/participant/PDPClient.h[m
[1mindex 85a0d3427..addda34ac 100644[m
[1m--- a/include/fastrtps/rtps/builtin/discovery/participant/PDPClient.h[m
[1m+++ b/include/fastrtps/rtps/builtin/discovery/participant/PDPClient.h[m
[36m@@ -49,6 +49,8 @@[m [mclass PDPClient : public PDP[m
     PDPClient(BuiltinProtocols* builtin);[m
     ~PDPClient();[m
 [m
[32m+[m[32m    private:[m[41m    [m
[32m+[m
     void initializeParticipantProxyData(ParticipantProxyData* participant_data) override;[m
 [m
     /**[m
[36m@@ -64,18 +66,6 @@[m [mclass PDPClient : public PDP[m
      */[m
     bool createPDPEndpoints() override;[m
 [m
[31m-    /**[m
[31m-     * Check if all servers have acknowledge the client PDP data[m
[31m-     * @return True if all can reach the client[m
[31m-     */[m
[31m-    bool all_servers_acknowledge_PDP();[m
[31m-[m
[31m-    /**[m
[31m-     * Check if we have our PDP received data updated[m
[31m-     * @return True if we known all the participants the servers are aware of[m
[31m-     */[m
[31m-    bool is_all_servers_PDPdata_updated();[m
[31m-[m
     /**[m
      * Force the sending of our local DPD to all servers [m
      * @param new_change If true a new change (with new seqNum) is created and sent; if false the last change is re-sent[m
[36m@@ -97,11 +87,21 @@[m [mclass PDPClient : public PDP[m
     void assignRemoteEndpoints(ParticipantProxyData* pdata) override;[m
     void removeRemoteEndpoints(ParticipantProxyData * pdata) override;[m
 [m
[32m+[m[32m    /**[m
[32m+[m[32m        * Check if all servers have acknowledge the client PDP data[m
[32m+[m[32m        * @return True if all can reach the client[m
[32m+[m[32m        */[m
[32m+[m[32m    bool all_servers_acknowledge_PDP();[m
[32m+[m
[32m+[m[32m    /**[m
[32m+[m[32m        * Check if we have our PDP received data updated[m
[32m+[m[32m        * @return True if we known all the participants the servers are aware of[m
[32m+[m[32m        */[m
[32m+[m[32m    bool is_all_servers_PDPdata_updated();[m
[32m+[m
     //!Matching server EDP endpoints[m
     void match_all_server_EDP_endpoints();[m
 [m
[31m-    private:[m
[31m-[m
     /**[m
     * TimedEvent for server synchronization: [m
     *   first stage: periodically resend the local RTPSParticipant information until all servers have acknowledge reception[m
[1mdiff --git a/thirdparty/idl b/thirdparty/idl[m
[1mindex 11e24c847..04930f792 160000[m
[1m--- a/thirdparty/idl[m
[1m+++ b/thirdparty/idl[m
[36m@@ -1 +1 @@[m
[31m-Subproject commit 11e24c84771855b64c6b2dc6fc6d63000ed71f11[m
[32m+[m[32mSubproject commit 04930f7926f972142811dd0dbdc0019356385e74[m
