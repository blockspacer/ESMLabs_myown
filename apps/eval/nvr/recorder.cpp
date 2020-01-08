#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/colorpicker.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <iostream>
#include <string>
#include <cstdint>
#include <memory>
#include <utility>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <mgnt_rtsp_client.h>
#include <mgnt_ff_muxer_v4.2.1.h>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::to_string;

class GLTexture
{
public:
    using handleType = std::unique_ptr<uint8_t[], void(*)(void*)>;
    GLTexture() = default;
    GLTexture(const std::string & textureName)
        : _textureName(textureName)
        , _textureId(0)
    {}

    GLTexture(const std::string & textureName, GLint textureId)
        : _textureName(textureName)
        , _textureId(textureId)
    {}

    GLTexture & operator=(const GLTexture & other) = delete;
    GLTexture & operator=(GLTexture && other) noexcept
    {
        _textureName = std::move(other._textureName);
        std::swap(_textureId, other._textureId);
        return (*this);
    }

    ~GLTexture(void) noexcept
    {
        if(_textureId)
        {
            glDeleteTextures(1, &_textureId);
        }
    }

    GLuint texture(void) const
    {
        return _textureId;
    }

    const std::string & textureName(void) const
    {
        return _textureName;
    }

    handleType load(const std::string & fileName)
    {
        if(_textureId)
        {
            ::glDeleteTextures(1, &_textureId);
            _textureId = 0;
        }
        int32_t force_channels = 0;
        int32_t w, h, n;
        handleType textureData(stbi_load(fileName.c_str(), &w, &h, &n, force_channels), stbi_image_free);
        if(!textureData)
            throw std::invalid_argument("Could not load texture data from file " + fileName);
        
        ::glGenTextures(1, &_textureId);
        ::glBindTexture(GL_TEXTURE_2D, _textureId);
        GLint internalFormat;
        GLint format;
        switch(n)
        {
            case 1:
                internalFormat = GL_R8;
                format = GL_RED;
                break;
            case 2:
                internalFormat = GL_RG8;
                format = GL_RG;
                break;
            case 3:
                internalFormat = GL_RGB8;
                format = GL_RGB;
                break;
            case 4:
                internalFormat = GL_RGBA8;
                format = GL_RGBA;
                break;
            default:
                internalFormat = 0;
                format = 0;
                break;
        }

        ::glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, textureData.get());
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        return textureData;
    }

private:
    std::string _textureName;
    GLuint _textureId;
};

class MGNTRecorder
    : public nanogui::Screen
    , public magnetar::lib::net::rtsp::client 
{
public:
    MGNTRecorder(void)
        : nanogui::Screen(Eigen::Vector2i(640, 480), "Magnetar NVR")
        , _rtsp_url(nullptr)
        , _play_btn(nullptr)
        , _stop_btn(nullptr)
        , _recv_first_video(false)
        , _recv_first_audio(false)
        , _end_video(false)
        , _end_audio(false)
    {
        nanogui::Window * window = new nanogui::Window(this, "RTSP Receiver");
        window->setPosition(Eigen::Vector2i(15, 15));

        nanogui::GridLayout * gridLayout = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 4, nanogui::Alignment::Middle, 10, 10);
        window->setLayout(gridLayout);

        new nanogui::Label(window, "URL : ");
        _rtsp_url = new nanogui::TextBox(window, "rtsp://192.168.18.201/Stream1");
        _rtsp_url->setEditable(true);
        _rtsp_url->setFixedWidth(250);

        _play_btn = new nanogui::Button(window, "play");
        _play_btn->setCallback([this]{
            on_click_play();                          
        });
        _stop_btn = new nanogui::Button(window, "stop");
        _stop_btn->setCallback([this]{
            on_click_stop();
        });

        performLayout();
    }

    ~MGNTRecorder(void)
    {

    }

    virtual bool keyboardEvent(int32_t key, int32_t scancode, int32_t action, int32_t modifiers)
    {
        if(nanogui::Screen::keyboardEvent(key, scancode, action, modifiers))
            return true;
        if(key==GLFW_KEY_ESCAPE && action==GLFW_PRESS)
        {
            setVisible(false);
            return true;
        }
        return false;
    }

    virtual void draw(NVGcontext * ctx)
    {

        nanogui::Screen::draw(ctx);
    }

    virtual void drawContents(void)
    {

    }

    void on_click_play(void)
    {
        std::string strRtspUrl = _rtsp_url->value();
        magnetar::lib::net::rtsp::client::play(strRtspUrl.c_str(), nullptr, nullptr, 
                                               magnetar::lib::net::rtsp::client::transport_option_t::rtp_over_tcp, 
                                               magnetar::lib::net::rtsp::client::media_type_t::video | magnetar::lib::net::rtsp::client::media_type_t::audio, 
                                               3000, 1.0f, false);
    }

    void on_click_stop(void)
    {
        magnetar::lib::net::rtsp::client::stop();
    } 

    virtual void on_begin_video(int32_t codec, uint8_t * vps, int32_t vpssize, uint8_t * sps, int32_t spssize, uint8_t * pps, int32_t ppssize, const uint8_t * bytes, int32_t nbytes, long long pts)
    {
	    _muxer.put_video_stream((uint8_t*)bytes, nbytes, pts, 0);

        int32_t index = 0;
        char extradata[MAX_PATH] = {0};
        if(vps && vpssize>0)
        {
            ::memmove(extradata, vps, vpssize);
            index += vpssize;
        }
        if(sps && spssize>0)
        {
            ::memmove(extradata + index, sps, spssize);
            index += spssize;
        }
        if(pps && ppssize>0)
        {
            ::memmove(extradata + index, pps, ppssize);
            index += ppssize;
        }

        _muxer_ctx.option |= magnetar::lib::container::ff::v4_2_1::muxer::media_type_t::video;
        ::strncpy(_muxer_ctx.path, "./muxing/tc201.mp4", sizeof(_muxer_ctx.path));
        _muxer_ctx.timestamp_mode = magnetar::lib::container::ff::v4_2_1::muxer::timestamp_mode_t::recved_time;
        _muxer_ctx.generation_rule = magnetar::lib::container::ff::v4_2_1::muxer::generation_rule_t::time;
        _muxer_ctx.generation_threshold = 1000;
        _muxer_ctx.video_codec = codec;
        _muxer_ctx.video_extradata_size = vpssize + spssize + ppssize;
        if(_muxer_ctx.video_extradata_size>0)
            ::memmove(_muxer_ctx.video_extradata, extradata, _muxer_ctx.video_extradata_size);
        _muxer_ctx.video_fps_num = 30000;
        _muxer_ctx.video_fps_den = 1001;
        _muxer_ctx.video_width = 1920;
        _muxer_ctx.video_height = 1080;
        _muxer_ctx.video_tb_num = 1;
        _muxer_ctx.video_tb_den = 90000;

        _recv_first_video = true;
        if(_recv_first_audio && _recv_first_video)
        {
            _muxer.initialize(&_muxer_ctx);
            _recv_first_audio = false;
            _recv_first_video = false;
        }
    }

    virtual void on_recv_video(int32_t codec, const uint8_t * bytes, int32_t nbytes, long long pts)
    {
        _muxer.put_video_stream((uint8_t*)bytes, nbytes, pts, 0);
    }

    virtual void on_end_video(void)
    {
        _end_video = true;
        if(_end_audio && _end_video)
        {
            _muxer.release();
            _end_video = false;
            _end_audio = false;
        }
    }

    virtual void on_begin_audio(int32_t codec, uint8_t * config, int32_t config_size, int32_t samplerate, int32_t channels, const uint8_t * bytes, int32_t nbytes, long long pts)
    {
        _muxer.put_audio_stream((uint8_t*)bytes, nbytes, pts, 0);

        _muxer_ctx.option |= magnetar::lib::container::ff::v4_2_1::muxer::media_type_t::audio;
        _muxer_ctx.audio_codec = codec;
        _muxer_ctx.audio_extradata_size = config_size;
        if(_muxer_ctx.audio_extradata_size > 0)
            ::memmove(_muxer_ctx.audio_extradata, config, _muxer_ctx.audio_extradata_size);
        _muxer_ctx.ignore_audio_extradata = true;
        _muxer_ctx.audio_frame_size = 1024;
        _muxer_ctx.audio_samplerate = samplerate;
        _muxer_ctx.audio_channels = channels < 2 ? 2 : channels;
        _muxer_ctx.audio_sampleformat = magnetar::lib::container::ff::v4_2_1::muxer::audio_sample_format_t::fmt_flt;
        _muxer_ctx.audio_tb_num = 1;
        _muxer_ctx.audio_tb_den = samplerate;

        _recv_first_audio = true;
        if(_recv_first_audio && _recv_first_video)
        {
            _muxer.initialize(&_muxer_ctx);
            _recv_first_audio = false;
            _recv_first_video = false;
        }
    }

    virtual void on_recv_audio(int32_t codec, const uint8_t * bytes, int32_t nbytes, long long pts)
    {
        _muxer.put_audio_stream((uint8_t*)bytes, nbytes, pts, 0);
    }

    virtual void on_end_audio(void)
    {
        _end_audio = true;
        if(_end_audio && _end_video)
        {
            _muxer.release();
            _end_video = false;
            _end_audio = false;
        }
    }    

 private:
    magnetar::lib::container::ff::v4_2_1::muxer::context_t _muxer_ctx;
    magnetar::lib::container::ff::v4_2_1::muxer _muxer;

    nanogui::TextBox * _rtsp_url;
    nanogui::Button * _play_btn;
    nanogui::Button * _stop_btn;
    bool _recv_first_video;
    bool _recv_first_audio;
    bool _end_video;
    bool _end_audio;
};

int32_t main(int32_t argc, char ** argv)
{
    try
    {
        nanogui::init();

        {
            nanogui::ref<MGNTRecorder> app = new MGNTRecorder();
            app->drawAll();
            app->setVisible(true);
            nanogui::mainloop();
        }

        nanogui::shutdown();
    }
    catch(const std::runtime_error & e)
    {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        std::cerr << error_msg << endl;
        return -1;
    }
    return 0;
}