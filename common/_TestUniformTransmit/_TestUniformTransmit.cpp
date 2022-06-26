#include "FrameFilterLib/Utility/FilterUniformTransmit.h"

#include <algorithm>
#include <thread>
#include "std-generic/cpplib/ThreadUtils.h"

using namespace ffl;

class TestSink : public AbstractFilter<AbstractSingleSourceSink> {
	int num_packets_to_receive_;
	std::vector<std::pair<std::chrono::steady_clock::time_point, size_t>> receive_data_;
public:
	TestSink(int num_packets_to_receive) : num_packets_to_receive_(num_packets_to_receive) {}

	const std::vector<std::pair<std::chrono::steady_clock::time_point, size_t>> receive_data() const { return receive_data_; }
	bool done() const {
		printf("\rReceived %.1f %%", 100 * float(receive_data_.size()) / num_packets_to_receive_);
		return receive_data_.size() == num_packets_to_receive_;
	}

	void ClearReceiveData() { receive_data_.clear(); }

	e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override {
		receive_data_.emplace_back(std::chrono::steady_clock::now(), buffer.size());
		return e_noResult;
	}

	bool IsCompatibleWith(const AbstractSink* sink) override { return true; }
};

void PrintAcc(const std::vector<size_t> &acc, int max, int y_step, int time_step_ms) {
	printf("bytes (step %u)\n", y_step);
	printf("  ^\n");
	for (int cur_cap = max; cur_cap >= 0; cur_cap -= y_step) {
		printf("  |");
		for (size_t s : acc) {
			putchar((s > cur_cap) ? '#' : ' ');
		}
		putchar('\n');
	}
	printf("  ");
	for (size_t s : acc) {
		putchar('-');
	}
	printf("-> time (step %i ms)\n", time_step_ms);
	fflush(stdout);
}

int main(int argc, char *argv[]) {
	size_t portion = 1200;
	size_t time_step_ms = 30;
	int send_pattern[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 18, 18, 18, 18, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

	printf("\n===============\n Input: \n===============\n\n");

	std::vector<size_t> acc;
	int max = 0;
	int sum1 = 0;
	for (int s : send_pattern) {
		acc.push_back(s * portion);
		max = std::max<int>(max, s * portion);
		sum1 += s * portion;
	}

	int max_instant_bitrate = max / time_step_ms;

	PrintAcc(acc, max, portion, time_step_ms);

	{
		auto test_sink = std::make_shared<TestSink>(sum1 / portion);
		auto uniform_transmit = std::make_shared<FilterUniformTransmit>(512000 / 8);
		uniform_transmit->RegisterSinkOrGetCompatible(test_sink);
		uniform_transmit->SetKorr(0.05);
		uniform_transmit->SetKorr2(2);
		uniform_transmit->SetN(1.5);

		for (int c : send_pattern) {
			for (int i = 0; i < c; i++) {
				uniform_transmit->PutFrame(nullptr, vs::SharedBuffer(portion), FrameMetadata());
			}
			vs::SleepFor(std::chrono::milliseconds(time_step_ms));
		}

		while (!test_sink->done()) {
			vs::SleepFor(std::chrono::milliseconds(time_step_ms));
		}

		auto receive_data = test_sink->receive_data();
		test_sink->ClearReceiveData();

		std::chrono::steady_clock::time_point t1 = receive_data.front().first,
										 t2 = receive_data.back().first;
		std::vector<size_t> acc(std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() / time_step_ms + 1);
		int sum2 = 0;
		for (auto &r : receive_data) {
			int i = std::chrono::duration_cast<std::chrono::milliseconds>(r.first - t1).count() / time_step_ms;
			acc[i] += r.second;
			sum2 += r.second;
		}

		assert(sum1 == sum2);

		printf("\n===============\n Output: \n===============\n\n");

		PrintAcc(acc, max, portion, time_step_ms);

		printf("\n===============\n Stats: \n===============\n\n");

		printf("%s\n\n", uniform_transmit->GetStatistics().c_str());
	}

	system("pause");
	return 0;
}

