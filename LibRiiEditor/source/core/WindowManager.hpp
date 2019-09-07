#pragma once

#include <queue>
#include <memory>
#include <mutex>

#include <variant>

#include "ui/Window.hpp"
#include "SelectionManager.hpp"

class WindowManager
{
public:
	void attachWindow(std::unique_ptr<Window> window);
	void detachWindow(u32 windowId);
	void processWindowQueue();
	void drawWindows(WindowContext* ctx = nullptr);

	SelectionManager& getSelectionManager()
	{
		return mSelectionManager;
	}
	const SelectionManager& getSelectionManager() const
	{
		return mSelectionManager;
	}

private:
	SelectionManager mSelectionManager;

	struct WindowQueueCommand
	{
		enum class Action
		{
			AttachWindow,
			DetachWindow
		};
		
		WindowQueueCommand(Action a, std::unique_ptr<Window> t)
			: action(a), target_a(std::move(t))
		{}
		WindowQueueCommand(Action a, u32 windowId_)
			: action(a), target_d(windowId_)
		{}

		Action getAction() const noexcept
		{
			return action;
		}

		std::unique_ptr<Window> getAttachmentTarget()
		{
			assert(action == Action::AttachWindow);
			
			return std::move(target_a);
			//return std::move(std::get<std::unique_ptr<Window>>(target));
		}

		u32 getDetachmentTarget()
		{
			assert(action == Action::DetachWindow);

			return target_d;
			//return std::get<u32>(target);
		}

	public:
		Action action;
		//	std::variant<std::unique_ptr<Window>, u32> target;
		std::unique_ptr<Window> target_a;
		u32 target_d;
	};

	struct WindowVector
	{
		// Actions themselves not atomic, lock the mutex before using
		std::mutex mutex;
		std::vector<std::unique_ptr<Window>> vector;

		void append(std::unique_ptr<Window> ptr)
		{
			vector.push_back(std::move(ptr));
		}

		void remove(u32 id)
		{
			auto it = std::find_if(std::begin(vector), std::end(vector),
				[id](const auto& element) { return element->mId == id; });

			assert(it != vector.end());
			if (it != vector.end())
				vector.erase(it);
		}
	};

	struct WindowQueue
	{
		std::mutex mutex;
		std::queue<WindowQueueCommand> queue;
	} mWindowQueue;

	// Must only be accessed from processWindowQueue
	u32 mWindowIdCounter = 0; // Don't decrement

protected:
	WindowVector mWindows;
};