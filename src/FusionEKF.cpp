#include "FusionEKF.h"
#include "tools.h"
#include "Eigen/Dense"
#include <iostream>

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;

/*
 * Constructor.
 */
FusionEKF::FusionEKF()
{
	is_initialized_ = false;

	previous_timestamp_ = 0;

	// Create a 2D state vector x
	ekf_.x_ = VectorXd(4);

	// State covariance matrix P
	ekf_.P_ = MatrixXd(4, 4);
	ekf_.P_ << 1, 0, 0, 0,
               0, 1, 0, 0,
               0, 0, 1000, 0,
               0, 0, 0, 1000;

	//measurement covariance matrix - laser
	R_laser_ = MatrixXd(2, 2);
	R_laser_ << 0.0225, 0.0000,
                0.0000, 0.0225;

	//measurement covariance matrix - radar
	R_radar_ = MatrixXd(3, 3);
	R_radar_ << 0.09, 0.0000, 0.00,
		        0.00, 0.0009, 0.00,
                0.00, 0.0000, 0.09;

	/**
	 TODO:
	 * Finish initializing the FusionEKF.
	 * Set the process and measurement noises
	 */

	//measurement matrix - laser
	H_laser_ = MatrixXd(2, 4);
	H_laser_ << 1, 0, 0, 0,
		        0, 1, 0, 0;

    // measurement matrix - radar
	Hj_ = MatrixXd(3, 4);

	// The initial state transition matrix F
	ekf_.F_ = MatrixXd(4, 4);
	ekf_.F_ << 1, 0, 1, 0,
               0, 1, 0, 1,
               0, 0, 1, 0,
               0, 0, 0, 1;
}

/**
* Destructor.
*/
FusionEKF::~FusionEKF() {}

void FusionEKF::ProcessMeasurement(const MeasurementPackage &measurement_pack) 
{
	double x = 0.0;  
	double y = 0.0;  

	if (measurement_pack.sensor_type_ == MeasurementPackage::LASER)
	{
	    x = measurement_pack.raw_measurements_(0);
	    y = measurement_pack.raw_measurements_(1);
	}
	else if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR)

	{
		/**
		Convert radar from polar to cartesian coordinates.
		*/
		double ro = measurement_pack.raw_measurements_(0);
		double phi = measurement_pack.raw_measurements_(1);

		x = ro * cos(phi);
		y = ro * sin(phi);
	}
	else
	{
		return;
	}


  /*****************************************************************************
   *  Initialization
   ****************************************************************************/
	if (!is_initialized_) 
	{
		/**
		TODO:
		  * Initialize the state ekf_.x_ with the first measurement.
		  * Create the covariance matrix.
		  * Remember: you'll need to convert radar from polar to cartesian coordinates.
		  */

		//set the state with the initial location and zero velocity
		ekf_.x_ << x, y, 0, 0;

		previous_timestamp_ = measurement_pack.timestamp_;

		// done initializing, no need to predict or update
		is_initialized_ = true;

		return;
	}

  /*****************************************************************************
   *  Prediction
   ****************************************************************************/

  /**
   TODO:
   * Update the state transition matrix F according to the new elapsed time.
     - Time is measured in seconds.
   * Update the process noise covariance matrix.
   * Use noise_ax = 9 and noise_ay = 9 for your Q matrix.
   */

    //compute the time elapsed between the current and previous measurements
	double dt = (measurement_pack.timestamp_ - previous_timestamp_) / 1000000.0;	//dt - expressed in seconds
	previous_timestamp_ = measurement_pack.timestamp_;

	//Modify the F matrix so that the time is integrated
	ekf_.F_(0, 2) = dt;
	ekf_.F_(1, 3) = dt;

	//set the process covariance matrix Q
	double dt_2 = dt * dt;
	double dt_3 = dt_2 * dt;
	double dt_4 = dt_3 * dt;

	const double noise_ax = 9;
	const double noise_ay = 9;

	ekf_.Q_ = MatrixXd(4, 4);
	ekf_.Q_ << dt_4 / 4 * noise_ax, 0, dt_3 / 2 * noise_ax, 0,
               0, dt_4 / 4 * noise_ay, 0, dt_3 / 2 * noise_ay,
               dt_3 / 2 * noise_ax, 0, dt_2*noise_ax, 0,
               0, dt_3 / 2 * noise_ay, 0, dt_2*noise_ay;

    ekf_.Predict();

	/*****************************************************************************
	 *  Update
	 ****************************************************************************/

	 /**
	  TODO:
		* Use the sensor type to perform the update step.
		* Update the state and covariance matrices.
	  */
	if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR)
	{
		// Radar updates
		this->Hj_ = tools.CalculateJacobian(ekf_.x_);

		ekf_.R_ = this->R_radar_;
		ekf_.H_ = this->Hj_;

		ekf_.UpdateEKF(measurement_pack.raw_measurements_);
	}
	else
	{
		// Laser updates
		ekf_.R_ = this->R_laser_;
		ekf_.H_ = this->H_laser_;

		ekf_.Update(measurement_pack.raw_measurements_);
	}

	// print the output
	cout << "x_ = " << ekf_.x_ << endl;
	cout << "P_ = " << ekf_.P_ << endl;
}
