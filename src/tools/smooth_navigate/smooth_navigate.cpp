#include "smooth_navigate.h"

namespace smooth_navigate
{

SmoothNavigate::SmoothNavigate()
{
    m_scroll.config.frequency = 240;
    m_scroll.config.base_step = 0.05;
    m_scroll.config.mod_factor = 2;
    m_scroll.config.ease_in = 0.25;
    m_scroll.config.ease_out = 0.12;

    m_move.config.frequency = 240;
    m_move.config.base_step = 1;     // Pixels
    m_move.config.mod_factor = 2.5;
    m_move.config.ease_in = 0.75;
    m_move.config.ease_out = 0; // No easing out

    m_scroll.state.progress = 0.0;
    m_scroll.state.step = m_scroll.config.base_step;

    m_move.state.progress = 0.0;
    m_move.state.step = m_move.config.base_step;


    m_keys = {
        {Event::ACTIVATE,      220},      // §
        {Event::ACTIVATE_MOD,  VK_MENU},
        {Event::LEFT_CLICK,    VK_SPACE},
        {Event::RIGHT_CLICK,   L'X'},
        {Event::MOVE_UP,       L'W'},
        {Event::MOVE_DOWN,     L'S'},
        {Event::MOVE_LEFT,     L'A'},
        {Event::MOVE_RIGHT,    L'D'},
        {Event::SCROLL_UP,     L'Q'},
        {Event::SCROLL_DOWN,   L'E'},
        {Event::SCROLL_LEFT,   L'J'},
        {Event::SCROLL_RIGHT,  L'L'},
        {Event::INCREASE,      L'R'},
        {Event::DECREASE,      L'F'},
        {Event::FAST_MODE,     L'C'},
        {Event::SLOW_MODE,     L'V'}
    };

    m_scroll.thread = std::thread(&SmoothNavigate::scroll_thread, this);
    m_move.thread = std::thread(&SmoothNavigate::move_thread, this);
}

SmoothNavigate::~SmoothNavigate()
{
    m_kill_threads = true;
    m_scroll.cv.notify_one();
    m_move.cv.notify_one();

    if (m_scroll.thread.joinable())
    {
        m_scroll.thread.join();
    }

    if (m_move.thread.joinable())
    {
        m_move.thread.join();
    }
}

int SmoothNavigate::run()
{
    return 0;
}

void SmoothNavigate::toggle(bool on)
{
    static int keyboard_id;

    if (on)
    {
        keyboard_id = LLInput::register_listener(
            WH_KEYBOARD_LL,
            CREATE_LISTENER(keyboard_hook_listener)
        );
    }
    else
    {
        LLInput::unregister_listener(WH_KEYBOARD_LL, keyboard_id);
    }
}

bool CALLBACK SmoothNavigate::keyboard_hook_listener(WPARAM w_param, LPARAM l_param)
{
    KBDLLHOOKSTRUCT* keydata = reinterpret_cast<KBDLLHOOKSTRUCT*>(l_param);
    WPARAM key = keydata->vkCode;

    // Check if key is in use
    bool valid_key = false;
    for (const auto& pair : m_keys)
    {
        if (pair.second == key)
        {
            valid_key = true;
            break;
        }
    }

    if (!valid_key)
        return false;

    // Process listed keys
    switch (w_param)
    {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:

        // Handle toggling
        if (key == m_keys[Event::ACTIVATE] && (HLInput::keydown(m_keys[Event::ACTIVATE_MOD])))
        {
            m_toggled_active = !m_toggled_active;
            if (m_toggled_active)
            {
                ::MessageBeep(MB_ICONASTERISK);   
            }
            else
            {
                ::MessageBeep(MB_ICONHAND);
            }
            return true;
        }
        else if (!m_toggled_active)
        {
            return false;
        }

        // Handle toggled on
        if (key == m_keys[Event::SCROLL_UP] ||
            key == m_keys[Event::SCROLL_DOWN] ||
            key == m_keys[Event::SCROLL_LEFT] ||
            key == m_keys[Event::SCROLL_RIGHT])
        {
                m_scroll.cv.notify_one();
                return true;
        }

        if (key == m_keys[Event::MOVE_UP] ||
            key == m_keys[Event::MOVE_DOWN] ||
            key == m_keys[Event::MOVE_LEFT] ||
            key == m_keys[Event::MOVE_RIGHT]
        )
        {
                POINT p;
                if (::GetCursorPos(&p))
                {
                    m_move.state.position.x = p.x;
                    m_move.state.position.y = p.y;
                }

                m_move.cv.notify_one(); // Wake yo ass up
                return true;
        }

        if (key == m_keys[Event::LEFT_CLICK])
        {
            if (!HLInput::keydown(m_keys[Event::LEFT_CLICK]))
            {
                HLInput::set_mouse(MK_LBUTTON, true);
            }

            return true;
        }

        if (key == m_keys[Event::RIGHT_CLICK])
        {
            if (!HLInput::keydown(m_keys[Event::RIGHT_CLICK]))
            {
                HLInput::set_mouse(MK_RBUTTON, true);
            }
            
            return true;
        }

        if (key == m_keys[Event::FAST_MODE] || key == m_keys[Event::SLOW_MODE])
        {            
            return true;
        }

        if (key == m_keys[Event::INCREASE] || key == m_keys[Event::DECREASE])
        {
            bool increase = key == m_keys[Event::INCREASE];
            m_move.state.step += increase ? m_move.config.base_step : -m_move.config.base_step;
            return true;
        }

        // Pass other keys
        return false;

    // If any valid keys are released while pressing the activator key block the release
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (!m_toggled_active)
        {
            return false;
        }
    
        if (key == m_keys[Event::LEFT_CLICK])
        {
            HLInput::set_mouse(MK_LBUTTON, false);
            return true;
        }
        
        if (key == m_keys[Event::RIGHT_CLICK])
        {
            HLInput::set_mouse(MK_RBUTTON, false);
            return true;
        }
        
        if (LLInput::keydown(m_keys[Event::ACTIVATE]))
        {
            return true;
        }

    // No blocking
    default:
        return false;
    }
}


void SmoothNavigate::scroll_thread()
{
    while (!m_kill_threads)
    {
        ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
        start_scroll();
        end_scroll();
        ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_LOWEST);

        {
            std::unique_lock<std::mutex> lock(m_scroll.mutex);
            m_scroll.cv.wait(lock, [&]() { return scrolling() || m_kill_threads; });
        }
    }
}

void SmoothNavigate::move_thread()
{
    while (!m_kill_threads)
    {
        ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
        start_move();
        ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_LOWEST);

        {
            std::unique_lock<std::mutex> lock(m_move.mutex);
            m_move.cv.wait(lock, [&]() { return moving() || m_kill_threads; });
        }
    }
}

void SmoothNavigate::start_scroll()
{
    m_scroll.reset_time();

    const double p0 = m_scroll.state.progress;
    double t_ease_in = m_scroll.config.ease_in - p0 * m_scroll.config.ease_in;

    while (scrolling() && m_toggled_active)
    {
        m_scroll.update();

        m_scroll.state.progress = p0 + std::min(m_scroll.state.dt / t_ease_in, 1 - p0);
        m_scroll.state.mod = std::pow(
            m_scroll.config.mod_factor,
            LLInput::keydown(m_keys[Event::FAST_MODE]) - LLInput::keydown(m_keys[Event::SLOW_MODE])
        );
        m_scroll.state.dir.x = LLInput::keydown(m_keys[Event::SCROLL_UP]) - LLInput::keydown(m_keys[Event::SCROLL_DOWN]);

        HLInput::scroll(
            easing::ease_in_out_sine(m_scroll.state.progress) * m_scroll.state.step *
            m_scroll.state.mod * m_scroll.state.dir.x, false
        );

        m_scroll.next_tick([&]() { return !scrolling(); });
    }
}

void SmoothNavigate::end_scroll()
{
    m_scroll.reset_time();

    const double p0 = m_scroll.state.progress;
    double t_ease_out = p0 * m_scroll.config.ease_out;

    auto& t0 = m_scroll.state.t0;
    auto& dt = m_scroll.state.dt;
    while (!scrolling() && m_scroll.state.dt < t_ease_out)
    {
        m_scroll.update();
        m_scroll.state.progress = p0 - std::min(dt / t_ease_out, p0);

        HLInput::scroll(
            easing::ease_out_sine(m_scroll.state.progress) * m_scroll.state.step *
            m_scroll.state.mod * m_scroll.state.dir.x, false
        );

        m_scroll.next_tick([&]() { return scrolling(); });
    }
}

void SmoothNavigate::start_move()
{
    m_move.reset_time();

    const double p0 = m_move.state.progress;
    double t_ease_in = (1 - p0) * m_move.config.ease_in;
    // double t_ease_in = m_move.config.ease_in - p0 * m_move.config.ease_in;

    auto& dt = m_move.state.dt;
    while (moving() && m_toggled_active)
    {
        m_move.update();

        m_move.state.progress = p0 + std::min(dt / t_ease_in, 1 - p0);
        m_move.state.mod = std::pow(m_move.config.mod_factor, LLInput::keydown(m_keys[Event::FAST_MODE]) - LLInput::keydown(m_keys[Event::SLOW_MODE])); 

        POINT p;
        if (::GetCursorPos(&p))
        {
            int dirx = LLInput::keydown(m_keys[Event::MOVE_RIGHT]) - LLInput::keydown(m_keys[Event::MOVE_LEFT]);
            int diry = LLInput::keydown(m_keys[Event::MOVE_DOWN]) - LLInput::keydown(m_keys[Event::MOVE_UP]);

            m_move.state.dir.x = dirx;
            m_move.state.dir.y = dirx;

            auto& pos = m_move.state.position;

            double normalize = std::abs(dirx) && std::abs(diry) ? 1 / 1.41421 : 1;

            pos.x += dirx * m_move.state.step * m_move.state.mod * normalize;
            pos.y += diry * m_move.state.step * m_move.state.mod * normalize;

            HLInput::move_cursor(pos.x, pos.y);
        }

        m_move.next_tick([&]() { return !moving(); });
    }
}

bool SmoothNavigate::scrolling()
{
    return LLInput::keys[m_keys[Event::SCROLL_UP]] || LLInput::keys[m_keys[Event::SCROLL_DOWN]];
}

bool SmoothNavigate::moving()
{
    return LLInput::keys[m_keys[Event::MOVE_UP]]   || LLInput::keys[m_keys[Event::MOVE_DOWN]] ||
           LLInput::keys[m_keys[Event::MOVE_LEFT]] || LLInput::keys[m_keys[Event::MOVE_RIGHT]];
}

} // namespace smooth_navigate