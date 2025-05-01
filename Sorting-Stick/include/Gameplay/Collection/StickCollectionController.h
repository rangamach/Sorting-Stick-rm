#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <thread>

namespace Gameplay
{
    namespace Collection
    {
        class StickCollectionView;
        class StickCollectionModel;
        struct Stick;
        enum class SortType;
        enum class SortState;

        class StickCollectionController
        {
        private:
            StickCollectionView* collection_view;
            StickCollectionModel* collection_model;

            std::vector<Stick*> sticks;
            SortType sort_type;

            std::thread sort_thread;

            int number_of_comparisons;
            int number_of_array_access;
            int current_operation_delay;

            int delay_in_ms;
            sf::String time_complexity;

            SortState sort_state;

            int color_delay;

            void initializeSticks();
            float calculateStickWidth();
            float calculateStickHeight(int array_pos);

            void updateStickPosition();
            void shuffleSticks();
            bool compareSticksByData(const Stick* a, const Stick* b) const;

            void resetSticksColor();
            void resetVariables();

            void processSortThreadState();

            void ProcessBubbleSort();

            void ProcessInsertionSort();
            
            void ProcessSelectionSort();
            
            void ProcessInPlaceMergeSort();
            void InPlaceMergeSort(int left, int right);
            void InPlaceMerge(int left, int middle, int right);
            void ProcessMergeSort();
            void MergeSort(int left, int right);
            void Merge(int left, int middle, int right);

            void ProcessQuickSort();
            int Partition(int left, int right);
            void QuickSort(int left, int right);


            void SetCompletedColor();
            bool isCollectionSorted();
            void destroy();

        public:
            StickCollectionController();
            ~StickCollectionController();

            void initialize();
            void update();
            void render();

            void reset();
            void sortElements(SortType sort_type);

            SortType getSortType();
            int getNumberOfComparisons();
            int getNumberOfArrayAccess();

            int getNumberOfSticks();
            int getDelayMilliseconds();
            sf::String getTimeComplexity();
        };
    }
}