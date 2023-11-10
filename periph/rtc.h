#ifndef PERIPH_RTC_H
#define PERIPH_RTC_H

#include "main.h"
#ifdef HAL_RTC_MODULE_ENABLED

#include "Core/Inc/rtc.h"
#include "etl/getter_setter.h"
#include "etl/function.h"
#include "etl/time.h"

namespace Project::periph {

    /// RTC peripheral class
    /// @note requirements: no RTC output
    class RealTimeClock {
        inline static const char days[7][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
        inline static const auto minimumUpdateInterval = etl::time::milliseconds(500);
        inline static auto lastUpdate = etl::Time(0);

        template <typename T>
        using Getter = etl::Getter<T, etl::Function<T(), RealTimeClock*>>;

        RTC_TimeTypeDef sTime = {};
        RTC_DateTypeDef sDate = {};

    public:
        /// default constructor
        constexpr RealTimeClock() = default;

        RealTimeClock(const RealTimeClock&) = delete; ///< disable copy constructor
        RealTimeClock& operator=(const RealTimeClock&) = delete;  ///< disable copy assignment

        /// get time and date and store to sTime and sDate
        void update() {
            auto now = etl::time::now();
            if (now - lastUpdate < minimumUpdateInterval) return;

            HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
            HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
            lastUpdate = now;
        }

        struct DateArgs { int weekDay, date, month, year; };
        void setDate(DateArgs args) {
            sDate.WeekDay = uint8_t(args.weekDay);
            sDate.Date = uint8_t(args.date);
            sDate.Month = uint8_t(args.month);
            sDate.Year = uint8_t(args.year);
            HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
        }

        struct TimeArgs { int hours, minutes, seconds; };
        void setTime(TimeArgs args) {
            sTime.Hours = args.hours;
            sTime.Minutes = args.minutes;
            sTime.Seconds = args.seconds;
            HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
        }

        const Getter<int> seconds   = {etl::bind<&RealTimeClock::getSeconds>(this)};
        const Getter<int> minutes   = {etl::bind<&RealTimeClock::getMinutes>(this)};
        const Getter<int> hours     = {etl::bind<&RealTimeClock::getHours>(this)};
        const Getter<int> weekDay   = {etl::bind<&RealTimeClock::getWeekDay>(this)};
        const Getter<int> date      = {etl::bind<&RealTimeClock::getDate>(this)};
        const Getter<int> month     = {etl::bind<&RealTimeClock::getMonth>(this)};
        const Getter<int> year      = {etl::bind<&RealTimeClock::getYear>(this)};
        const Getter<const char*> day = {etl::bind<&RealTimeClock::getDay>(this)};

    private:
        int getSeconds()  { update(); return sTime.Seconds; }
        int getMinutes()  { update(); return sTime.Minutes; }
        int getHours()    { update(); return sTime.Hours; }
        int getWeekDay()  { update(); return sDate.WeekDay; }
        int getDate()     { update(); return sDate.Date; }
        int getMonth()    { update(); return sDate.Month; }
        int getYear()     { update(); return sDate.Year; }

        const char* getDay() { update(); return days[sDate.WeekDay]; }
    };

    inline RealTimeClock rtc;

} // namespace Project

#endif // HAL_RTC_MODULE_ENABLED
#endif // PERIPH_RTC_H