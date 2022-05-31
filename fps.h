#pragma once
#include <chrono>
#include <ctime>

class Interval
{
public:
	Interval() noexcept
		: start{ std::chrono::steady_clock::now() }
	{
	}

	auto value() const noexcept
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

class FpsCounter
{
public:
	FpsCounter() noexcept
		: fps{ 60 }
		, frameCount{ 0 }
		, timer{ Interval() }
	{
	}

	void tick() noexcept
	{
		frameCount++;

		if (timer.value() >= 1000000)
		{
			const auto alpha = 0.25; // TODO: consider making this configurable
			fps = alpha * fps + (1.0 - alpha) * frameCount;
			//fps = frameCount; // TODO: consider separating average fps and exact fps

			frameCount = 0;
			timer.reset();
		}
	}

	auto getFps() const noexcept
	{
		return fps;
	}

	auto getFrameCount() const noexcept
	{
		return frameCount;
	}

private:
	double fps;
	unsigned int frameCount;
	Interval timer;
};
