#include <cstdlib>
#include <cassert>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <array>

#define MAX_ROUNDS 4
#define MAX_ROUND_QUESTIONS 10
#define MAX_ANSWERS 10
#ifndef ROUNDS_PATH
#	define ROUNDS_PATH "rounds/"
#endif

class GameShow
{
 public:
	GameShow(std::string name)
		: name { name }
	{}

	const std::string name {};
};

class FamilyFeud : public GameShow
{
	int points {};

 public:
	FamilyFeud()
		: GameShow { "FEUD" }
	{}

	int get_points() const
	{
		return points;
	}

	void reset_points()
	{
		points = 0;
	}

	virtual void award_points(int points)
	{
		this->points += points;
	}
};

class FeudTest : FamilyFeud
{
	struct Answer
	{
		std::string answer {};
		int points { 0 };
		bool is_picked { false };
	};

	struct Question
	{
		std::string question {};
		std::array<Answer, MAX_ANSWERS> answers {};
		int answer_count {};
	};

	struct Round
	{
		std::array<Question, MAX_ROUND_QUESTIONS> questions {};
		int question_count {};
	};

	enum class End
	{
		Normal, FastMoney
	};

	const static int NUM_ROUNDS { 4 };
	std::array<Round, NUM_ROUNDS> rounds {};
	int current_round {};
	int strikes {};

	const std::string contestant_name {};

 public:
	FeudTest(std::string contestant_name_)
		: contestant_name { contestant_name_ }
	{}

	bool load_questions()
	{
		/// load the questions and answers for each round
		int r = 0; // the current round index 
		for( ; r < MAX_ROUNDS; ++r)
		{
			std::string round_path { ROUNDS_PATH + std::to_string(r+1) + "/" };
			std::ifstream questions_file { round_path + "Questions.txt" };

			assert(questions_file.is_open());
			int q = 0; // the current question index 
			for (std::string line; 
				std::getline(questions_file, line) && rounds[r].question_count < MAX_ROUND_QUESTIONS; 
				++rounds[r].question_count, ++q) 
			{
				std::istringstream iss_line { line };
				std::string answer_filename {};
				std::string question {};

				iss_line >> answer_filename;      // the first string is the answers filename
				std::getline(iss_line, question); // the rest of the line is the question
				rounds[r].questions[q].question = question;

				/// load the answers to the current question
				std::ifstream answers_file { round_path + answer_filename };
				assert(answers_file.is_open());
				int a = 0; // the current answer index
				for (std::string line_; 
					std::getline(answers_file, line_) && rounds[r].questions[q].answer_count < MAX_ANSWERS; 
					++rounds[r].questions[q].answer_count, ++a) 
				{
					std::istringstream iss_line { line_ };
					std::string answer {};
					int points {};

					iss_line >> answer;
					iss_line >> points;
					rounds[r].questions[q].answers[a].answer = answer;
					rounds[r].questions[q].answers[a].points = points;
				}
			}
		}
		return true;
	}

	void reset_answers()
	{
		for(auto round : rounds)
		{
			for(int q = 0; q < round.question_count; ++q)
			{
				for(int a = 0; a < round.questions[q].answer_count; ++a)
				{
					round.questions[q].answers[a].is_picked = false;
				}
			}
		}
	}

	void restart()
	{
		FamilyFeud::reset_points();
		this->reset_answers();
		srand(static_cast<unsigned int>(time(0)));
		std::cout << "WELCOME " << contestant_name << "!, let's get ready to play " << GameShow::name << "!" << std::endl;
		int round_points = 0;
		int point_multiplier = 1;
		for(int r = 0; r < MAX_ROUNDS; ++r)
		{
			Round & current_round = rounds[r];
			assert(current_round.question_count > 0 && current_round.question_count < MAX_ROUND_QUESTIONS);
			const int random_question_index = rand() % current_round.question_count;
			Question & current_question = current_round.questions[random_question_index]; // select a random question from this round
			strikes = 0;
			switch(r) // set point multiplier based on round
			{
				case 0:
				case 1:
				default:
					point_multiplier = 1; // for round 1 and 2
					break;
				case 2:
					point_multiplier = 2; // for round 3 
					break;
				case 3:
					point_multiplier = 3; // for round 4
					break;
			}
			round_points = 0; // points specific to this round
			std::cout << "\nROUND " << (r + 1) << std::endl;
			std::cout << "TOP " << (current_question.answer_count) << " ANSWERS ON THE BOARD: ";
			std::cout << current_question.question << ":" << std::endl;
			int total_correct_answers = 0;
			while(strikes < 3)
			{
				std::cout << "\nAnswer: ";
				std::string answer;
				std::cin >> answer;
				std::transform(answer.begin(), answer.end(), answer.begin(), std::toupper);
				bool correct_answer = false;
				int a = 0;
				for(; a < current_question.answer_count; ++a)
				{
					if(answer == current_question.answers[a].answer)
					{
						if(!current_question.answers[a].is_picked)
							correct_answer = true;
						break;
					}
				}
				if(correct_answer)
				{
					++total_correct_answers;
					current_question.answers[a].is_picked = true;
					int new_points = current_question.answers[a].points * point_multiplier;
					round_points += new_points;
					FamilyFeud::award_points(round_points);
					std::cout << "CORRECT ANSWER!!! " << (new_points) << " points" << std::endl;
					if(total_correct_answers >= current_question.answer_count)
					{
						std::cout << "\nCONGRATULATIONS YOU SWEPT THE BOARD, ROUND " << (r + 1) << " IS OVER" << std::endl;
						std::cout << "Total Points for Round " << (r) << ": " << round_points << std::endl;
						if(r < MAX_ROUNDS - 1)
							std::cout << contestant_name << ", get ready for ROUND " << (r + 1) << std::endl;
						break;
					}
				}
				else
				{
					++strikes;
					std::cout << "STRIKE " << strikes << std::endl;
					if(strikes >= 3) 
					{
						std::cout << "\nSORRY YOU HAVE 3 STRIKES, ROUND " << (r + 1) << " IS OVER" << std::endl;
						std::cout << "Total Points for Round " << (r + 1) << ": " << round_points << std::endl;
						if(r < MAX_ROUNDS - 1)
							std::cout << contestant_name << ", get ready for ROUND " << (r + 2) << std::endl;
						break; // next round
					}
				}
			}
		}
		std::cout << "\nGAME TOTAL: " << FamilyFeud::get_points() << std::endl;
		if(FamilyFeud::get_points() < 500)
			std::cout << "\nSORRY " << contestant_name << ", you do not have enough points to move to the Fast Money Round" << std::endl;
		else
			std::cout << "\nCONGRATULATIONS!!! " << contestant_name << ", you move on to the FAST MONEY ROUND!!!" << std::endl;
	}

};

int main()
{
	std::string contestant_name;
	std::cout << "Enter name of contestant: ";
	std::cin >> contestant_name;
	FeudTest test_game { contestant_name };
	assert(test_game.load_questions());
	test_game.restart();
}