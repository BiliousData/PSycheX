/*  NOTICE!
    This code was originally made by MaxDev, and is here for reference to translate it to C.
    If either BiliousData or NintendoBro385 manage to forget to credit him, please contact one of us to fix that.
    The rest below is the original unmodified lua script.
*/

-- I'll try to make the code as clean as possible

-- First, defining some variables we'll use for our timer.
total_time = 2500 -- in centiseconds
converted_time = 0 -- from centiseconds to seconds
timer_tick = 0 -- idk how to explain, refer to the code
timer_total = 0 -- Non-visual timer, counting in seconds
timer_visual = {0, 0} -- seconds, minutes.

while true do -- Infinite cycle. Classic.
    buttons.read() -- Reading button's input

    game_init() -- Some shit. Don't pay attention.

    converted_time = total_time / 100 -- That's how we got seconds from centiseconds.

    if timer_total < converted_time then -- if we still can add,
        timer_tick = timer_tick + 1 -- As the game adds 1 to it every frame, we have to reset it as soon as it reaches 60.

        if timer_tick == 60 then -- 1 second.
            timer_total = timer_total + 1 -- adding 1 to total timer
            timer_visual[1] = timer_visual[1] + 1 -- adding 1 to visual timer
            timer_tick = 0 -- resetting our timer_tick.
        end

        if timer_visual[1] == 60 then -- If visual timer's seconds is 60, we reset it and add 1 to visual minutes.
            timer_visual[1] = 0
            timer_visual[2] = timer_visual[2] + 1
        end
        
        if timer_visual[1] < 10 then -- Trying to avoid cases like "1:5"
            screen.print(0, 120, timer_visual[2] .. ":0" .. timer_visual[1], 1.6) -- Printing visual timer with a 0
        else
            screen.print(0, 120, timer_visual[2] .. ":" .. timer_visual[1], 1.6) -- Printing visual timer without a 0
        end
    else -- but if finished,
        if timer_visual[1] < 10 then -- Trying to avoid cases like "1:5"
            screen.print(0, 120, "Timer's out at " .. timer_visual[2] .. ":0" .. timer_visual[1], 1.2) -- Telling the player that the timer is over with a 0.
        else
            screen.print(0, 120, "Timer's out at " .. timer_visual[2] .. ":" .. timer_visual[1], 1.2) -- Telling the player that the timer is over without a 0.
        end
    end

    screen.print(0, 0, "Timer demo.\nGiven time (in centiseconds): " .. total_time .. "; \nConverted time (in seconds): " .. converted_time .. "\nTimer tick: " .. timer_tick .. "\nGlobal Timer: " .. timer_total)

    screen.flip() -- Flipping the screen.

    -- Voila!
end