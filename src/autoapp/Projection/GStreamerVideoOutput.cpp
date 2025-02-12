#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <f1x/openauto/autoapp/Projection/IVideoOutput.hpp>
#include <f1x/openauto/autoapp/Projection/VideoOutput.hpp>

namespace f1x {
namespace openauto {
namespace autoapp {
namespace projection {

class GStreamerVideoOutput : public VideoOutput
{
public:
    GStreamerVideoOutput(configuration::IConfiguration::Pointer configuration)
        : VideoOutput(std::move(configuration))
        , pipeline_(nullptr)
        , appsrc_(nullptr)
    {
    }

    virtual ~GStreamerVideoOutput() { stop(); }

    bool open() override
    {
        // Initialize GStreamer (if not already done)
        gst_init(nullptr, nullptr);

        // Create the pipeline and elements.
        pipeline_ = gst_pipeline_new("video-pipeline");
        appsrc_ = gst_element_factory_make("appsrc", "video-source");
        GstElement* h264parse = gst_element_factory_make("h264parse", "h264-parser");
        GstElement* decoder = gst_element_factory_make("avdec_h264", "h264-decoder");
        GstElement* videoconvert = gst_element_factory_make("videoconvert", "converter");
        GstElement* videosink = gst_element_factory_make("autovideosink", "video-output");

        if (!pipeline_ || !appsrc_ || !h264parse || !decoder || !videoconvert || !videosink)
        {
            // Log error: failed to create GStreamer elements.
            return false;
        }

        // Configure appsrc properties.
        GstCaps* caps = gst_caps_new_simple("video/x-h264",
                                             "stream-format", G_TYPE_STRING, "avc",
                                             "alignment", G_TYPE_STRING, "au",
                                             NULL);
        g_object_set(G_OBJECT(appsrc_), "caps", caps, NULL);
        gst_caps_unref(caps);

        // Add all elements to the pipeline.
        gst_bin_add_many(GST_BIN(pipeline_), appsrc_, h264parse, decoder, videoconvert, videosink, NULL);
        if (!gst_element_link_many(appsrc_, h264parse, decoder, videoconvert, videosink, NULL))
        {
            // Log error: pipeline linking failed.
            return false;
        }

        // Start playing the pipeline.
        GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_PLAYING);
        return (ret != GST_STATE_CHANGE_FAILURE);
    }

    bool init() override
    {
        // Any additional initialization if needed.
        return true;
    }

    void write(uint64_t timestamp, const aasdk::common::DataConstBuffer& buffer) override
    {
        // Create a new GstBuffer to hold the video data.
        GstBuffer *gstBuffer = gst_buffer_new_allocate(nullptr, buffer.size, nullptr);
        gst_buffer_fill(gstBuffer, 0, buffer.cdata, buffer.size);

        // Optionally set timestamp (conversion to GstClockTime may be needed).
        GST_BUFFER_PTS(gstBuffer) = timestamp;

        // Push the buffer into appsrc.
        GstFlowReturn flowReturn = gst_app_src_push_buffer(GST_APP_SRC(appsrc_), gstBuffer);
        if (flowReturn != GST_FLOW_OK)
        {
            // Log error: buffer push failed.
        }
    }

    void stop() override
    {
        if (pipeline_)
        {
            gst_element_set_state(pipeline_, GST_STATE_NULL);
            gst_object_unref(GST_OBJECT(pipeline_));
            pipeline_ = nullptr;
            appsrc_ = nullptr;
        }
    }

private:
    GstElement* pipeline_;
    GstElement* appsrc_;
};

} // namespace projection
} // namespace autoapp
} // namespace openauto
} // namespace f1x
