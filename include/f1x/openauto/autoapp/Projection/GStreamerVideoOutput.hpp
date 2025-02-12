#pragma once

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <f1x/openauto/autoapp/Projection/IVideoOutput.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <QRect>
#include <memory>

namespace f1x {
namespace openauto {
namespace autoapp {
namespace projection {

class GStreamerVideoOutput : public IVideoOutput
{
public:
    using Pointer = std::shared_ptr<GStreamerVideoOutput>;

    // The constructor accepts a configuration pointer even though we won't call OMX-specific methods.
    explicit GStreamerVideoOutput(configuration::IConfiguration::Pointer configuration);
    virtual ~GStreamerVideoOutput();

    // IVideoOutput interface implementations
    bool open() override;
    bool init() override;
    void write(uint64_t timestamp, const aasdk::common::DataConstBuffer& buffer) override;
    void stop() override;

    // Return default video parameters.
    aasdk::proto::enums::VideoFPS::Enum getVideoFPS() const override;
    aasdk::proto::enums::VideoResolution::Enum getVideoResolution() const override;
    size_t getScreenDPI() const override;
    QRect getVideoMargins() const override;

private:
    configuration::IConfiguration::Pointer configuration_;

    // GStreamer elements.
    GstElement* pipeline_;
    GstElement* appsrc_;

    bool isActive_;
};

} // namespace projection
} // namespace autoapp
} // namespace openauto
} // namespace f1x
