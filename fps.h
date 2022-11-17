#pragma once
#include <memory>

class Timer;

class FpsCounter
{
public:
	FpsCounter();
	void tick() noexcept;
	[[nodiscard]] double getFps() const noexcept;
	[[nodiscard]] double getAverageFps() const noexcept;
	[[nodiscard]] unsigned int getFrameCount() const noexcept;

private:
	double fps, averageFps;
	unsigned int frameCount;
	std::shared_ptr<Timer> timer;
};
