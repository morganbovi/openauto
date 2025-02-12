#include <f1x/openauto/autoapp/Projection/GStreamerVideoOutput.hpp>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <iostream>
#include <cstring>

namespace f1x {
namespace openauto {
namespace autoapp {
namespace projection {

GStreamerVideoOutput::GStreamerVideoOutput(configuration::IConfiguration::Pointer configuration)
    : configuration_(std::move(configuration))
    , pipeline_(nullptr)
    , appsrc_(nullptr)
    , isActive_(false)
{
}

GStreamerVideoOutput::~GStreamerVideoOutput()
{
    stop();
    if (pipeline_) {
        gst_object_unref(GST_OBJECT(pipeline_));
        pipeline_ = nullptr;
    }
}

bool GStreamerVideoOutput::open()
{
    // Initialize GStreamer (gst_init is safe to call repeatedly)
    gst_init(nullptr, nullptr);

    // Create the pipeline using a simple launch string.
    // This pipeline uses an appsrc element to accept H.264 data,
    // then parses, decodes, converts, and renders via autovideosink.
    const char* pipelineDesc = "appsrc name=video_src ! h264parse ! avdec_h264 ! videoconvert ! autovideosink";
    GError* error = nullptr;
    pipeline_ = gst_parse_launch(pipelineDesc, &error);
    if (!pipeline_) {
        std::cerr << "Failed to create GStreamer pipeline: "
                  << (error ? error->message : "unknown error") << std::endl;
        if (error)
            g_error_free(error);
        return false;
    }

    // Retrieve the appsrc element from the pipeline.
    appsrc_ = gst_bin_get_by_name(GST_BIN(pipeline_), "video_src");
    if (!appsrc_) {
        std::cerr << "Failed to retrieve appsrc element from the pipeline." << std::endl;
        return false;
    }

    // Set appsrc properties (caps and format).
    GstCaps* caps = gst_caps_new_simple("video/x-h264",
                                        "stream-format", G_TYPE_STRING, "avc",
                                        "alignment", G_TYPE_STRING, "au",
                                        nullptr);
    g_object_set(G_OBJECT(appsrc_), "caps", caps, "format", GST_FORMAT_TIME, nullptr);
    gst_caps_unref(caps);

    // Set the pipeline to PLAYING.
    GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        std::cerr << "Unable to set the GStreamer pipeline to PLAYING state." << std::endl;
        return false;
    }

    isActive_ = true;
    return true;
}

bool GStreamerVideoOutput::init()
{
    // No additional initialization needed for this simple example.
    return true;
}

void GStreamerVideoOutput::write(uint64_t timestamp, const aasdk::common::DataConstBuffer& buffer)
{
    if (!isActive_ || !appsrc_)
        return;

    // Create a new GstBuffer sized to hold the data.
    GstBuffer* gstBuffer = gst_buffer_new_allocate(nullptr, buffer.size, nullptr);
    if (!gstBuffer) {
        std::cerr << "Failed to allocate GstBuffer." << std::endl;
        return;
    }

    // Copy the data from the input buffer into the GstBuffer.
    gst_buffer_fill(gstBuffer, 0, buffer.cdata, buffer.size);

    // Set the presentation timestamp.
    // Assuming 'timestamp' is in microseconds; convert to nanoseconds for GStreamer.
    GST_BUFFER_PTS(gstBuffer) = timestamp * 1000;
    // Set a default duration (for example, for 30 FPS).
    GST_BUFFER_DURATION(gstBuffer) = GST_SECOND / 30;

    // Push the buffer into the appsrc.
    GstFlowReturn flowRet = gst_app_src_push_buffer(GST_APP_SRC(appsrc_), gstBuffer);
    if (flowRet != GST_FLOW_OK) {
        std::cerr << "Error pushing buffer into appsrc: " << flowRet << std::endl;
    }
}

void GStreamerVideoOutput::stop()
{
    if (isActive_ && pipeline_) {
        gst_element_set_state(pipeline_, GST_STATE_NULL);
        isActive_ = false;
    }
}

// --- Default implementations for video parameter getters ---
// Since we're not using OMX or any hardware-specific configuration here,
// we return constant defaults. Adjust these defaults as needed.

aasdk::proto::enums::VideoFPS::Enum GStreamerVideoOutput::getVideoFPS() const
{
    // Default to 30 FPS.
    return aasdk::proto::enums::VideoFPS::FPS_30;  // Adjust this value if your enum differs.
}

aasdk::proto::enums::VideoResolution::Enum GStreamerVideoOutput::getVideoResolution() const
{
    // Default to HD720.
    return aasdk::proto::enums::VideoResolution::HD720;  // Adjust as needed.
}

size_t GStreamerVideoOutput::getScreenDPI() const
{
    // Default DPI.
    return 96;
}

QRect GStreamerVideoOutput::getVideoMargins() const
{
    // No margins by default.
    return QRect();
}

} // namespace projection
} // namespace autoapp
} // namespace openauto
} // namespace f1x
