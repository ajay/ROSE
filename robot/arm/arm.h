#include <armadillo>
#include <string>
#include <thread>
#include <mutex>
#include <njson/json.hpp>
#include <sys/time.h>
#include "baserobot.h"

class Arm : public BaseRobot
{
	public:
		Arm(void);
		~Arm(void);

		bool connect(void);
		bool connected(void);
		int numConnected(void);
		void disconnect(void);
		void send(
				const arma::vec &arm_theta,
				const arma::vec &arm_vel,
				bool arm_theta_act = false,
				bool arm_vel_act = true);
		arma::vec recv(void);
		void reset(void);

		void load_calibration_params(const std::string &filename);
		void set_calibration_params(nlohmann::json cp);
		bool calibrated(void);

		/* update on recv */
		arma::vec arm_read;
		arma::vec arm_fback;
		arma::vec arm_current;
		arma::vec arm_pos;
		/* parameters */
		arma::vec arm_mint;
		arma::vec arm_maxt;
		arma::vec arm_minv;
		arma::vec arm_maxv;
		arma::vec arm_link_length;

		/* threaded versions */
		void move(
				const arma::vec &arm_theta,
				const arma::vec &arm_vel,
				bool arm_theta_act = false,
				bool arm_vel_act = true);
		arma::vec sense(void);

		void set_pose(
				double joint0,
				double joint1,
				double joint2,
				double joint3,
				double joint4,
				double joint5,
				bool en = true);

		// Technically slower, but lets you set one at a time
		void set_joint(double joint_val, int joint_id);

		arma::vec get_end_effector_pos(int linkid = 6); // change to DOF later
		bool get_position_placement(
				arma::vec target_pos,
				arma::vec target_pose,
				double target_spin,
				arma::vec &solution_enc);

	private:
		std::thread *devmgr;
		std::mutex rlock;
		std::mutex wlock;
		bool manager_running;

		void update_uctrl(void);
		void update_send();
		void update_recv(void);

		arma::vec buffered_arm_theta;
		arma::vec buffered_arm_vel;
		bool buffered_arm_theta_en;
		bool buffered_arm_vel_en;
		arma::vec buffered_arm_sensors;
		struct timeval prevwtime;

		bool calibration_loaded;
};