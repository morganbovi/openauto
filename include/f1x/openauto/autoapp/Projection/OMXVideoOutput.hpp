#pragma once

#include <f1x/openauto/autoapp/Projection/IVideoOutput.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <memory>

// You might also include other necessary headers (like GStreamer headers)
// #include <gst/gst.h>
// #include <gst/app/gstappsrc.h>

namespace f1x {
namespace openauto {
namespace autoapp {
namespace projection {

class GStreamerVideoOutput : public IVideoOutput
{
public:
    // Define a type alias for a smart pointer, if you follow that convention.
    using Pointer = std::shared_ptr<GStreamerVideoOutput>;

    // Constructor that accepts a configuration pointer.
    explicit GStreamerVideoOutput(configuration::IConfiguration::Pointer configuration);

    // Virtual destructor.
    virtual ~GStreamerVideoOutput();

    // IVideoOutput interface implementation
    bool open() override;
    bool init() override;
    void write(uint64_t timestamp, const aasdk::common::DataConstBuffer& buffer) override;
    void stop() override;

private:
    // Private members (for example, pointers to your GStreamer pipeline, appsrc, etc.)
    configuration::IConfiguration::Pointer configuration_;

    // Example: GStreamer pipeline pointer
    GstElement* pipeline_;
    GstElement* appsrc_;

    // Other internal state variables as needed...
};

} // namespace projection
} // namespace autoapp
} // namespace openauto
} // namespace f1x
