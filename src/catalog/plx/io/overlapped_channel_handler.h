//#~def plx::OverlappedChannelHandler
///////////////////////////////////////////////////////////////////////////////
// plx::OverlappedChannelHandler
//
namespace plx {

class OverlappedChannelHandler {
public:
  virtual void OnConnect(plx::OverlappedContext* ovc, unsigned long error) = 0;
  virtual void OnRead(plx::OverlappedContext* ovc, unsigned long error) = 0;
  virtual void OnWrite(plx::OverlappedContext* ovc, unsigned long error) = 0;
};

}
