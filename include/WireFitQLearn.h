#ifndef WIREFITQLEARN_H
#define WIREFITQLEARN_H

#include <vector>

#include "Trainer.h"
#include "Learner.h"

namespace net{
	class NeuralNet;
}

namespace rl {

	class Interpolator;
	struct Wire;

	/** A Learner using Q-learning that works with continous state action spaces (Gaskett et al.).
	 *
	 * A wire fitted interpolator function is used in conjunction with a neural network to extend Q-Learning to continuous actions and state vectors.
	 */
	class WireFitQLearn : public Learner {
	public:
		net::NeuralNet *network;
		Interpolator *interpolator;
		net::Trainer *trainer;
		unsigned int numberOfWires, actionDimensions;
		double learningRate, devaluationFactor;
		double controlPointsGDErrorTarget, controlPointsGDLearningRate;
		int controlPointsGDMaxIterations;
		unsigned int baseOfDimensions;
		State lastState;
		Action minAction, maxAction, lastAction;

		/** Initializes a completely new WireFitQLearn object with all necesary values.
		*
		* \param stateDimensions the dimensions of the state vector being fed to the system
		* \param actionDimensions_ the dimensions of the action vector that will be outputted by the system
		* \param numHiddenLayers the number of hidden layers of the neural network that will be approximating the reward values of each action
		* \param numNeuronsPerHiddenLayer the number of neurons per hidden layer of the neural network that will be approximating the reward values of each action
		* \param numberOfWires_ the number of multi-dimensional data-points or "wires" that will be outputted by the neural network. Wires are interpolated so that a continuous function of action versus expected reward may be generated by the system. The more complex the task, the more wires are needed.
		* \param minAction_ the minimum action vector that the system may output
		* \param maxAction_ the maximum action vector that the system may output
		* \param baseOfDimensions_ the number of possible descrete values in each dimension. Ex. if baseOfDimensions=2, minAction={0, 0}, maxAction={1, 1}, possibleActions={{0, 0}, {0, 1}, {1, 0}, {1, 1}}.
		* \param interpolator_ the object that will interpolate the data-points or "wires" of action versus reward that the neural network will output
		* \param trainer_ the model that will train the neural network which is fed the state and outputs data-points or "wires" of action versus reward
		* \param learningRate_ a constant between 0 and 1 that dictates how fast the robot learns from reinforcement.
		* \param devaluationFactor_ a constant between 0 and 1 that weighs future reward vs immediate reward. A value of 0 will make the network only value immediate reward, while a value of 1 will make it consider future reward with the same weight as immediate reward.
		*/
		WireFitQLearn(unsigned int stateDimensions, unsigned int actionDimensions_, unsigned int numHiddenLayers, unsigned int numNeuronsPerHiddenLayer, unsigned int numberOfWires_, Action minAction_, Action maxAction_, unsigned int baseOfDimensions_, Interpolator *interpolator_, net::Trainer *trainer_, double learningRate_, double devaluationFactor_);

		/** Initializes an empty, non-valid WireFitQLearn object. */
		WireFitQLearn();

		/** Gets the action that the network deems most benificial for the current state
		 *
		 * \param currentState a vector of doubles representing the "inputs" or sensor values of the system
		 */
		Action chooseBestAction(State currentState);

		/** Gets an action using the Boltzman softmax probability distribution
		 *
		 * Non-random search heuristic used so that the neural network explores actions despite their reward value. The lower the exploration constanstant, the more likely it is to pick the best action for the current state.
		 * \param currentState a vector of doubles representing the "inputs" or sensor values of the system
		 * \param explorationConstant a positive floating point number representing the exploration level of the system. Common values range from 0.01 to 1. The higher this number is, the more likely it is that the system will pick worse actions.
		 */
		Action chooseBoltzmanAction(State currentState, double explorationConstant);

		/** Updates expected reward values.
		 *
		 * Given the immediate reward from the last action taken and the new state, this function updates the correct value for the longterm reward of the lastAction and trains the network in charge of the lastAction to output the corect reward value.
		 * \param reward the reward value from the last action
		 * \param newState the new state (aka. inputs) to the system
		 */
		void applyReinforcementToLastAction(double reward, State newState);

		/** Resets the system's model so that to a newely initialized state */
		void reset();

	protected:
		// Feeds the state into the network, parses to the output of the network into wire form, and outputs these wires
		std::vector<Wire> getWires(State state);

		/*
		 * Gets the number of wires specified on the interpolator function for the given state between the min and max actions given.
		 * The number of wires returned is baseOfDimensions raised to the power of the number of actionDimensions (the size of minAction or maxAction).
		 */
		std::vector<Wire> getSetOfWires(const State &state,
										int baseOfDimensions);

		// Given a set of wires converts them to the raw output of the NN
		std::vector<double> getRawOutput(std::vector<Wire> wires);

		// Gets the highest reward value possible for a given state
		double highestReward(State state);

		// Gets the action with the highest reward value for a given state
		Action bestAction(State state);

		// Gets the q value of an action
		double getQValue(double reward,
			const State &oldState,
			const State &newState,
			const Action &action,
			const std::vector<Wire> &controlWires);

		// Using gradient descent, outputs a new set of control wires using a new "correct" wire and the old control wires
		std::vector<Wire> newControlWires(const Wire &correctWire, std::vector<Wire> controlWires);
	};
};

#endif
