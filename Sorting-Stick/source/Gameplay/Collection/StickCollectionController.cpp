#include "Gameplay/Collection/StickCollectionController.h"
#include "Gameplay/Collection/StickCollectionView.h"
#include "Gameplay/Collection/StickCollectionModel.h"
#include "Gameplay/GameplayService.h"
#include "Global/ServiceLocator.h"
#include "Gameplay/Collection/Stick.h"
#include <random>

namespace Gameplay
{
	namespace Collection
	{
		using namespace UI::UIElement;
		using namespace Global;
		using namespace Graphics;

		StickCollectionController::StickCollectionController()
		{
			collection_view = new StickCollectionView();
			collection_model = new StickCollectionModel();

			for (int i = 0; i < collection_model->number_of_elements; i++) sticks.push_back(new Stick(i));
		}

		StickCollectionController::~StickCollectionController()
		{
			destroy();
		}

		void StickCollectionController::initialize()
		{
			this->sort_state = SortState::NotSorting;
			collection_view->initialize(this);
			initializeSticks();
			reset();
		}

		void StickCollectionController::initializeSticks()
		{
			float rectangle_width = calculateStickWidth();


			for (int i = 0; i < collection_model->number_of_elements; i++)
			{
				float rectangle_height = calculateStickHeight(i); //calc height

				sf::Vector2f rectangle_size = sf::Vector2f(rectangle_width, rectangle_height);

				sticks[i]->stick_view->initialize(rectangle_size, sf::Vector2f(0, 0), 0, collection_model->element_color);
			}
		}

		void StickCollectionController::update()
		{
			processSortThreadState();
			collection_view->update();
			for (int i = 0; i < sticks.size(); i++) sticks[i]->stick_view->update();
		}

		void StickCollectionController::render()
		{
			collection_view->render();
			for (int i = 0; i < sticks.size(); i++) sticks[i]->stick_view->render();
		}

		float StickCollectionController::calculateStickWidth()
		{
			float total_space = static_cast<float>(ServiceLocator::getInstance()->getGraphicService()->getGameWindow()->getSize().x);

			// Calculate total spacing as 10% of the total space
			float total_spacing = collection_model->space_percentage * total_space;

			// Calculate the space between each stick
			float space_between = total_spacing / (collection_model->number_of_elements - 1);
			collection_model->setElementSpacing(space_between);

			// Calculate the remaining space for the rectangles
			float remaining_space = total_space - total_spacing;

			// Calculate the width of each rectangle
			float rectangle_width = remaining_space / collection_model->number_of_elements;

			return rectangle_width;
		}

		float StickCollectionController::calculateStickHeight(int array_pos)
		{
			return (static_cast<float>(array_pos + 1) / collection_model->number_of_elements) * collection_model->max_element_height;
		}

		void StickCollectionController::updateStickPosition()
		{
			for (int i = 0; i < sticks.size(); i++)
			{
				float x_position = (i * sticks[i]->stick_view->getSize().x) + ((i + 1) * collection_model->elements_spacing);
				float y_position = collection_model->element_y_position - sticks[i]->stick_view->getSize().y;

				sticks[i]->stick_view->setPosition(sf::Vector2f(x_position, y_position));
			}
		}

		void StickCollectionController::UpdateStickPosition(int i)
		{
			float x_pos = (i * sticks[i]->stick_view->getSize().x) + ((i)*collection_model->elements_spacing);
			float y_pos = collection_model->element_y_position - sticks[i]->stick_view->getSize().y;

			sticks[i]->stick_view->setPosition(sf::Vector2f(x_pos, y_pos));
		}

		void StickCollectionController::shuffleSticks()
		{
			std::random_device device;
			std::mt19937 random_engine(device());

			std::shuffle(sticks.begin(), sticks.end(), random_engine);
			updateStickPosition();
		}

		bool StickCollectionController::compareSticksByData(const Stick* a, const Stick* b) const
		{
			return a->data < b->data;
		}

		void StickCollectionController::processSortThreadState()
		{
			if (sort_thread.joinable() && isCollectionSorted())
			{
				sort_thread.join(); 
				this->sort_state = SortState::NotSorting;
			}
		}

		void StickCollectionController::ProcessBubbleSort()
		{
			Sound::SoundService* sound_service = ServiceLocator::getInstance()->getSoundService();

			time_complexity = "O(n^2)";
			int i, j;
			for (i = 0; i < sticks.size(); i++)
			{
				if (sort_state == SortState::NotSorting) break;
				bool swapped = false;
				for (j = 1; j < sticks.size() - i; j++)
				{
					if (sort_state == SortState::NotSorting) break;

					number_of_array_access += 2;
					number_of_comparisons++;

					sound_service->playSound(Sound::SoundType::COMPARE_SFX);

					sticks[j - 1]->stick_view->setFillColor(collection_model->processing_element_color);
					sticks[j]->stick_view->setFillColor(collection_model->processing_element_color);

					if (sticks[j - 1]->data > sticks[j]->data)
					{
						std::swap(sticks[j - 1], sticks[j]);
						swapped = true;
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));

					sticks[j - 1]->stick_view->setFillColor(collection_model->element_color);
					sticks[j]->stick_view->setFillColor(collection_model->element_color);
					updateStickPosition();
				}

				if (sticks.size() - i - 1 >= 0)
					sticks[sticks.size() - i - 1]->stick_view->setFillColor(collection_model->placement_position_element_color);
				if (!swapped)
					break;
			}
			SetCompletedColor();
		}

		void StickCollectionController::ProcessInsertionSort()
		{
			Sound::SoundService* sound_service = ServiceLocator::getInstance()->getSoundService();
			time_complexity = "O(n^2)";

			int i;
			for (i = 1; i < sticks.size(); ++i)
			{
				if (sort_state == SortState::NotSorting) break;

				int j = i - 1;
				Stick* stick_key = sticks[i];
				number_of_array_access++;

				stick_key->stick_view->setFillColor(collection_model->processing_element_color);

				std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));

				while (j >= 0 && sticks[j]->data > stick_key->data)
				{
					if (sort_state == SortState::NotSorting) break;

					number_of_comparisons++;
					number_of_array_access++;

					sticks[j + 1] = sticks[j];
					number_of_array_access++;
					
					sticks[j + 1]->stick_view->setFillColor(collection_model->processing_element_color);
					j--;

					sound_service->playSound(Sound::SoundType::COMPARE_SFX);
					updateStickPosition();

					std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));

					sticks[j + 2]->stick_view->setFillColor(collection_model->selected_element_color);
				}
				sticks[j + 1] = stick_key;
				number_of_array_access++;

				sticks[j + 1]->stick_view->setFillColor(collection_model->temporary_processing_color);
				sound_service->playSound(Sound::SoundType::COMPARE_SFX);
				updateStickPosition();

				std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));
				sticks[j + 1]->stick_view->setFillColor(collection_model->selected_element_color);
			}
			SetCompletedColor();
		}

		void StickCollectionController::ProcessSelectionSort()
		{
			Sound::SoundService* sound_service = ServiceLocator::getInstance()->getSoundService();
			time_complexity = "O(n^2)";

			int i;
			for (i = 0; i < sticks.size() - 1; ++i)
			{
				if (sort_state == SortState::NotSorting) break;
				int min_index = i;
				sticks[i]->stick_view->setFillColor(collection_model->selected_element_color);
				int j;
				for (j = i + 1; j < sticks.size(); ++j)
				{
					if (sort_state == SortState::NotSorting) break;
					
					number_of_array_access+=2;
					number_of_comparisons++;

					sound_service->playSound(Sound::SoundType::COMPARE_SFX);
					sticks[j]->stick_view->setFillColor(collection_model->processing_element_color);
					std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));

					if (sticks[j]->data < sticks[min_index]->data)
					{
						if (min_index != i)
							sticks[min_index]->stick_view->setFillColor(collection_model->element_color);
						min_index = j;
						sticks[min_index]->stick_view->setFillColor(collection_model->processing_element_color);
					}
					else
						sticks[j]->stick_view->setFillColor(collection_model->element_color);
				}
				number_of_array_access += 3;
				std::swap(sticks[min_index], sticks[i]);

				sticks[i]->stick_view->setFillColor(collection_model->placement_position_element_color);
				updateStickPosition();
			}
			sticks[sticks.size() - 1]->stick_view->setFillColor(collection_model->placement_position_element_color);

			SetCompletedColor();
		}

		void StickCollectionController::ProcessInPlaceMergeSort()
		{
			time_complexity = "O(nLog(n))";
			InPlaceMergeSort(0, sticks.size() - 1);
			SetCompletedColor();
		}

		void StickCollectionController::InPlaceMergeSort(int left, int right)
		{
			if (left < right)
			{
				int middle = left + (right - left) / 2;

				InPlaceMergeSort(left, middle);
				InPlaceMergeSort(middle + 1, right);

				InPlaceMerge(left, middle, right);
			}
		}

		void StickCollectionController::InPlaceMerge(int left, int middle, int right)
		{
			Sound::SoundService* sound_service = ServiceLocator::getInstance()->getSoundService();

			int start = middle + 1;

			if (sticks[middle]->data <= sticks[start]->data)
			{
				number_of_comparisons++;
				number_of_array_access += 2;
				
				return;
			}

			while (left <= middle && start <= right)
			{
				number_of_comparisons++;
				number_of_array_access += 2;

				if (sticks[left]->data <= sticks[start]->data)
					left++;
				else
				{
					Stick* value = sticks[start];
					int index = start;
					while (index != left)
					{
						sticks[index] = sticks[index - 1];
						index--;
						number_of_array_access += 2;
					}
					sticks[left] = value;
					number_of_array_access++;

					left++;
					middle++;
					start++;

					updateStickPosition();
				}
				sound_service->playSound(Sound::SoundType::COMPARE_SFX);
				sticks[left - 1]->stick_view->setFillColor(collection_model->processing_element_color);
				std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));
				sticks[left - 1]->stick_view->setFillColor(collection_model->element_color);
			}
		}

		void StickCollectionController::ProcessMergeSort()
		{
			time_complexity = "O(nLog(n))";

			MergeSort(0, sticks.size() - 1);
			SetCompletedColor();
		}

		void StickCollectionController::MergeSort(int left, int right)
		{
			if (left >= right)
				return;
			int middle = left + (right - left) / 2;

			MergeSort(left, middle);
			MergeSort(middle + 1, right);

			Merge(left, middle, right);
		}

		void StickCollectionController::Merge(int left, int middle, int right)
		{
			Sound::SoundService* sound_service = ServiceLocator::getInstance()->getSoundService();

			std::vector<Stick*> temp(right - left + 1);
			for (int i = left; i <= right; ++i)
			{
				temp[i - left] = sticks[i];
				number_of_array_access++;
				sticks[i]->stick_view->setFillColor(collection_model->temporary_processing_color);
				updateStickPosition();
			}

			int i = 0;
			int j = middle - left + 1;
			int k = left;

			while (i <= middle - left && j <= right - left)
			{
				number_of_comparisons++;
				number_of_array_access += 2;

				if (temp[i]->data <= temp[j]->data)
				{
					sticks[k++] = temp[i++];
				}
				else
				{
					sticks[k++] = temp[j++];
				}

				sound_service->playSound(Sound::SoundType::COMPARE_SFX);
				sticks[k - 1]->stick_view->setFillColor(collection_model->processing_element_color);
				updateStickPosition();
				std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));
			}

			while (i <= middle - left)
			{
				number_of_array_access++;
				sticks[k++] = temp[i++];
				sound_service->playSound(Sound::SoundType::COMPARE_SFX);
				sticks[k - 1]->stick_view->setFillColor(collection_model->processing_element_color);
				updateStickPosition();
				std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));
			}

			while (j <= right - left)
			{
				number_of_array_access++;
				sticks[k++] = temp[j++];
				sound_service->playSound(Sound::SoundType::COMPARE_SFX);
				sticks[k - 1]->stick_view->setFillColor(collection_model->processing_element_color);
				updateStickPosition();
				std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));
			}
		}

		void StickCollectionController::ProcessQuickSort()
		{
			time_complexity = "O(nLog(n))";

			QuickSort(0, sticks.size() - 1);

			SetCompletedColor();
		}

		int StickCollectionController::Partition(int left, int right)
		{
			Sound::SoundService* sound_service = ServiceLocator::getInstance()->getSoundService();

			sticks[right]->stick_view->setFillColor(collection_model->selected_element_color);

			int i = left - 1;
			int j;
			for (j = left; j < right; ++j)
			{
				sticks[j]->stick_view->setFillColor(collection_model->processing_element_color);
				number_of_array_access += 2;
				number_of_comparisons++;

				if (sticks[j]->data < sticks[right]->data)
				{
					++i;
					std::swap(sticks[i], sticks[j]);
					number_of_array_access += 3;
					sound_service->playSound(Sound::SoundType::COMPARE_SFX);

					updateStickPosition();
					std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));
				}
				sticks[j]->stick_view->setFillColor(collection_model->element_color);
			}
			std::swap(sticks[i + 1], sticks[right]);
			number_of_array_access += 3;

			updateStickPosition();

			return i + 1;

		}

		void StickCollectionController::QuickSort(int left, int right)
		{
			if (left < right)
			{
				int pivot_index = Partition(left, right);

				QuickSort(left, pivot_index - 1);
				QuickSort(pivot_index + 1, right);

				int i;
				for (i = left; i <= right; i++)
				{
					sticks[i]->stick_view->setFillColor(collection_model->placement_position_element_color);
					updateStickPosition();
				}
			}
		}

		void StickCollectionController::ProcessRadixSort()
		{
			RadixSort();

			SetCompletedColor();
		}

		void StickCollectionController::CountSort(int exponent)
		{
			Sound::SoundService* sound_service = Global::ServiceLocator::getInstance()->getSoundService();

			std::vector<Stick*> output_array(sticks.size());
			std::vector<int> count(10, 0);
			int i;
			for (i = 0; i < sticks.size(); ++i) {
				sound_service->playSound(Sound::SoundType::COMPARE_SFX);
				int digit = (sticks[i]->data / exponent) % 10;
				count[digit]++;
				number_of_array_access++;
				sticks[i]->stick_view->setFillColor(collection_model->processing_element_color);
				std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay / 2));
				sticks[i]->stick_view->setFillColor(collection_model->element_color);
			}

			for (i = 1; i < 10; ++i) {
				count[i] += count[i - 1];
			}

			for (i = sticks.size() - 1; i >= 0; --i) {

				int digit = (sticks[i]->data / exponent) % 10;
				output_array[count[digit] - 1] = sticks[i];
				output_array[count[digit] - 1]->stick_view->setFillColor(collection_model->temporary_processing_color);
				count[digit]--;
				number_of_array_access++;

			}

			for (i = 0; i < sticks.size(); ++i) 
			{
				sticks[i] = output_array[i];
				sticks[i]->stick_view->setFillColor(collection_model->placement_position_element_color);
				UpdateStickPosition(i);
				std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));
			}
		}

		void StickCollectionController::RadixSort()
		{
			int max_element = INT_MIN;
			const int size = sticks.size();

			int i;
			for (i = 0; i < size; ++i)
				max_element = std::max(sticks[i]->data, max_element);
			for (i = 1; max_element / i > 0; i *= 10) 
				CountSort(i);
		}

		void StickCollectionController::resetSticksColor()
		{
			for (int i = 0; i < sticks.size(); i++) sticks[i]->stick_view->setFillColor(collection_model->element_color);
		}

		void StickCollectionController::resetVariables()
		{
			number_of_comparisons = 0;
			number_of_array_access = 0;
		}

		void StickCollectionController::reset()
		{
			current_operation_delay = 0;
			color_delay = 0;
			this->sort_state = SortState::NotSorting;
			if (sort_thread.joinable()) sort_thread.join();

			shuffleSticks();
			resetSticksColor();
			resetVariables();
		}

		void StickCollectionController::sortElements(SortType sort_type)
		{
			current_operation_delay = collection_model->operation_delay;
			this->sort_state = SortState::Sorting;
			this->sort_type = sort_type;

			switch (sort_type)
			{
			case Gameplay::Collection::SortType::BUBBLE_SORT:
				sort_thread = std::thread(&StickCollectionController::ProcessBubbleSort, this);
				break;
			case Gameplay::Collection::SortType::INSERTION_SORT:
				sort_thread = std::thread(&StickCollectionController::ProcessInsertionSort, this);
				break;
			case Gameplay::Collection::SortType::SELECTION_SORT:
				sort_thread = std::thread(&StickCollectionController::ProcessSelectionSort, this);
				break;
			case Gameplay::Collection::SortType::MERGE_SORT:
				sort_thread = std::thread(&StickCollectionController::ProcessMergeSort, this);
				break;
			case Gameplay::Collection::SortType::QUICK_SORT:
				sort_thread = std::thread(&StickCollectionController::ProcessQuickSort, this);
				break;
			case Gameplay::Collection::SortType::RADIX_SORT:
				sort_thread = std::thread(&StickCollectionController::ProcessRadixSort, this);
				break;
			}
		}

		void StickCollectionController::SetCompletedColor()
		{
			Sound::SoundService* sound_service = ServiceLocator::getInstance()->getSoundService();

			int i;
			for (i = 0; i < sticks.size(); i++)
			{
				if (sort_state == SortState::NotSorting) break;
				sticks[i]->stick_view->setFillColor(collection_model->element_color);
			}
			for (i = 0; i < sticks.size(); i++)
			{
				if (sort_state == SortState::NotSorting) break;
				sound_service->playSound(Sound::SoundType::COMPARE_SFX);
				sticks[i]->stick_view->setFillColor(collection_model->placement_position_element_color);

				std::this_thread::sleep_for(std::chrono::milliseconds(color_delay));
			}
			if (sort_state == SortState::Sorting)
				sound_service->playSound(Sound::SoundType::Scream);
		}

		bool StickCollectionController::isCollectionSorted()
		{
			for (int i = 1; i < sticks.size(); i++) if (sticks[i] < sticks[i - 1]) return false;
			return true;
		}

		void StickCollectionController::destroy()
		{
			current_operation_delay = 0;
			if (sort_thread.joinable()) sort_thread.join();

			for (int i = 0; i < sticks.size(); i++) delete(sticks[i]);
			sticks.clear();

			delete (collection_view);
			delete (collection_model);
		}

		SortType StickCollectionController::getSortType() { return sort_type; }

		int StickCollectionController::getNumberOfComparisons() { return number_of_comparisons; }

		int StickCollectionController::getNumberOfArrayAccess() { return number_of_array_access; }

		int StickCollectionController::getNumberOfSticks() { return collection_model->number_of_elements; }

		int StickCollectionController::getDelayMilliseconds() { return current_operation_delay; }

		sf::String StickCollectionController::getTimeComplexity() { return time_complexity; }
	}
}


