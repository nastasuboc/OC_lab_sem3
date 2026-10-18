#include "pch.h"
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <vector>
#include "marker.h"

class MarkerTest : public ::testing::Test {
protected:
	std::vector<HANDLE> threads;

	void SetUp() override {
		InitializeCriticalSection(&cs);
		startEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		blockedEvent.clear();
		continueEvent.clear();
		exitEvent.clear();
		threads.clear();
		arr = nullptr;
	}

	void Prepare(int size, int k) {
		n = size;
		arr = new int[n];
		for (int i = 0; i < n; ++i) arr[i] = 0;
		for (int i = 0; i < k; ++i) {
			blockedEvent.push_back(CreateEvent(NULL, TRUE, FALSE, NULL));
			continueEvent.push_back(CreateEvent(NULL, FALSE, FALSE, NULL));
			exitEvent.push_back(CreateEvent(NULL, TRUE, FALSE, NULL));
		}
		for (int i = 0; i < k; ++i) {
			threads.push_back(CreateThread(NULL, 0, marker, (LPVOID)(INT_PTR)(i + 1), 0, NULL));
		}
	}

	bool WaitAllBlocked() {
		return WaitForMultipleObjects((DWORD)blockedEvent.size(), blockedEvent.data(), TRUE, 5000) == WAIT_OBJECT_0;
	}

	void TearDown() override {
		SetEvent(startEvent);
		for (HANDLE e : exitEvent) SetEvent(e);
		if (!threads.empty())
			WaitForMultipleObjects((DWORD)threads.size(), threads.data(), TRUE, 5000);
		for (HANDLE t : threads) CloseHandle(t);
		for (HANDLE e : blockedEvent) CloseHandle(e);
		for (HANDLE e : continueEvent) CloseHandle(e);
		for (HANDLE e : exitEvent) CloseHandle(e);
		CloseHandle(startEvent);
		DeleteCriticalSection(&cs);
		delete[] arr;
		arr = nullptr;
	}
};

TEST_F(MarkerTest, SingleMarkerMarksCellAndBlocks) {
	Prepare(1, 1);
	SetEvent(startEvent);
	ASSERT_TRUE(WaitAllBlocked());
	EXPECT_EQ(arr[0], 1);
}

TEST_F(MarkerTest, ExitSignalClearsMarkedCells) {
	Prepare(1, 1);
	SetEvent(startEvent);
	ASSERT_TRUE(WaitAllBlocked());
	SetEvent(exitEvent[0]);
	EXPECT_EQ(WaitForSingleObject(threads[0], 5000), WAIT_OBJECT_0);
	EXPECT_EQ(arr[0], 0);
}

TEST_F(MarkerTest, ExitOfOneMarkerKeepsMarksOfOthers) {
	Prepare(10, 2);
	SetEvent(startEvent);
	ASSERT_TRUE(WaitAllBlocked());
	std::vector<int> before(arr, arr + n);
	SetEvent(exitEvent[0]);
	ASSERT_EQ(WaitForSingleObject(threads[0], 5000), WAIT_OBJECT_0);
	for (int i = 0; i < n; ++i) {
		EXPECT_NE(arr[i], 1);
		if (before[i] == 2) EXPECT_EQ(arr[i], 2);
	}
}

TEST_F(MarkerTest, ContinueSignalKeepsMarkerAlive) {
	Prepare(1, 1);
	SetEvent(startEvent);
	ASSERT_TRUE(WaitAllBlocked());
	ResetEvent(blockedEvent[0]);
	SetEvent(continueEvent[0]);
	ASSERT_TRUE(WaitAllBlocked());
	EXPECT_EQ(WaitForSingleObject(threads[0], 0), WAIT_TIMEOUT);
	EXPECT_EQ(arr[0], 1);
}

TEST_F(MarkerTest, MarkerDoesNotWorkBeforeStartSignal) {
	Prepare(5, 1);
	Sleep(100);
	for (int i = 0; i < n; ++i) {
		EXPECT_EQ(arr[i], 0);
	}
}

TEST(PrintArrayTest, PrintsAllElements) {
	n = 3;
	arr = new int[3] { 1, 0, 2 };
	std::ostringstream out;
	std::streambuf* old = std::cout.rdbuf(out.rdbuf());
	printArray();
	std::cout.rdbuf(old);
	EXPECT_EQ(out.str(), "1 0 2 \n");
	delete[] arr;
	arr = nullptr;
}