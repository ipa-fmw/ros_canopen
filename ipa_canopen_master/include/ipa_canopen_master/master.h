#ifndef H_IPA_CANOPEN_MASTER
#define H_IPA_CANOPEN_MASTER

#include <ipa_canopen_master/canopen.h>

namespace ipa_canopen{

class SyncLayer: public Layer, public SyncCounter{
public:
    SyncLayer(const ipa_can::Header &h, const boost::posix_time::time_duration &p, const uint8_t &o) : SyncCounter(h,p,o) {}
};

class Master: boost::noncopyable{
public:
    virtual boost::shared_ptr<SyncLayer> getSync(const ipa_can::Header &h, const boost::posix_time::time_duration &t, const uint8_t overflow) = 0;
    virtual ~Master() {}
};
    
class LocalSyncLayer:  public SyncLayer{
public:
    LocalSyncLayer(const ipa_can::Header &h, const boost::posix_time::time_duration &p, const uint8_t &o, boost::shared_ptr<ipa_can::CommInterface> interface, bool loopback);
    
    virtual bool read();
    virtual bool write();
    virtual bool report();
    virtual bool init();
    virtual bool recover();
    virtual bool shutdown();
    
private:
    boost::mutex mutex_;
    boost::condition_variable cond;
    
    boost::shared_ptr<ipa_can::CommInterface> interface_;
    ipa_can::Frame msg_;
    bool loopback_;
    ipa_can::CommInterface::FrameListener::Ptr loop_listener_;

    boost::posix_time::time_duration timeout;
    boost::posix_time::time_duration max_timeout;
    boost::posix_time::time_duration track_timeout;
    
    Timer timer_;

    void handleFrame(const ipa_can::Frame & msg);

    bool checkSync();
    bool sync();
};

class LocalMaster: public Master{
    boost::mutex mutex_;
    boost::unordered_map<ipa_can::Header, boost::shared_ptr<LocalSyncLayer> > layers_;
    boost::shared_ptr<ipa_can::CommInterface> interface_;
    const bool loopback_;
public:
    boost::shared_ptr<SyncLayer> getSync(const ipa_can::Header &h, const boost::posix_time::time_duration &t, const uint8_t overflow);
    template<typename Driver> LocalMaster(boost::shared_ptr<Driver> interface): interface_(interface), loopback_(interface->doesLoopBack()) {}
};

} // ipa_canopen
#endif // !H_IPA_CANOPEN_MASTER