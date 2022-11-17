#include "fps.h"
#include <chrono>

class Timer
{
public:
	Timer() noexcept
		: start{ std::chrono::steady_clock::now() }
	{
	}

	[[nodiscard]] auto value() const noexcept
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count();
	}

	void reset() noexcept
	{
		start = std::chrono::steady_clock::now();
	}

private:
	std::chrono::steady_clock::time_point start;
};

FpsCounter::FpsCounter()
	: fps{ 60 }
	, averageFps{ 60 }
	, frameCount{ 0 }
	, timer{ new Timer }
{
}

void FpsCounter::tick() noexcept
{
	frameCount++;

	if (timer->value() >= 1000000)
	{
		fps = frameCount;

		constexpr auto alpha = 0.25; // TODO: consider making this configurable
		averageFps = alpha * averageFps + (1.0 - alpha) * frameCount;

		frameCount = 0;
		timer->reset();
	}
}

double FpsCounter::getFps() const noexcept
{
	return fps;
}

double FpsCounter::getAverageFps() const noexcept
{
	return fps;
}

unsigned int FpsCounter::getFrameCount() const noexcept
{
	return frameCount;
}
