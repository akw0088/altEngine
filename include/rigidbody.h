#include "include.h"

#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#define MAX_VELOCITY 800.0f


typedef struct
{
	bool escort;
	float speed;
	int wait;
	float path_min_dist;
} train_t;

class RigidBody : public Model
{
public:
	RigidBody(Entity *entity);
	void recalc();
	virtual void integrate(float time);
	bool collision_detect(vec3 &v);
	bool collision_detect(Plane &p);
	bool collision_detect_simple(RigidBody &body);
	bool collision_detect(RigidBody &body);
	bool collision_distance(RigidBody &body);
	void impulse(Plane &plane, vec3 &vertex);
	void impulse(RigidBody &rigid, vec3 &point);
	void impulse(RigidBody &rigid, vec3 &point, Plane &plane);
	void frame2ent(Frame *camera_frame, input_t &input);
	void frame2ent_yaw(Frame *camera_frame, input_t &input);
	void save_config(cfg_t &config);
	void load_config(cfg_t &config);
	void get_frame(Frame &frame);

	//AI
	void seek(const vec3 &position);
	void flee(const vec3 &position);
	void arrive(const vec3 &position);
	void pursue();
	void evade();
	void wander(float radius, float distance, float jitter);
	void set_target(Entity &target);


	bool move(input_t &input, float speed_scale);
	bool ground_move(input_t &input, float speed_scale);
	bool air_move(input_t &input, float speed_scale);
	bool water_move(input_t &input, float speed_scale);
	bool flight_move(input_t &input, float speed_scale);

	void move_forward(float speed_scale);
	void move_backward(float speed_scale);
	void move_left(float speed_scale);
	void move_right(float speed_scale);
	void move_up(float speed_scale); // jump
	void move_down(float speed_scale);
	void lookat(vec3 &target);
	void lookat_yaw(vec3 &target);
	void pid_follow_path(vec3 *path_list, int num_path, float max_velocity, float distance, int wait);
	int train_follow_path(vec3 *target, float max_velocity, float distance, int wait);

	float get_volume();
	float get_height();
	float *get_matrix(float *matrix);


	unsigned int impact_index;

	bool			on_ground;
	bool			hard_impact;
	float			impact_velocity;
	bool			pursue_flag;
	bool			step_flag;
	Entity			*target;
	vec3			sphere_target;
	int			jump_timer;
	unsigned int	bounce;


	float			restitution;
	bool			sleep;
	int				y_offset;
	bool			gravity;
	bool			noclip;
	bool			flight;
	bool			water;
	bool			last_water;
	bool			lava;
	bool			slime;
	int				step_type;
	int				bsp_trigger_volume;
	int				bsp_model_platform;
	bool			rotational_friction_flag;
	bool			translational_friction_flag;
	bool			ground_friction_flag;
	float			rotational_friction;
	float			translational_friction;
	float			water_depth;
	//Physical
	float			mass;
	matrix3			inverse_tensor;
	matrix3			world_tensor;
	vec3			velocity;
	vec3			angular_velocity;
	vec3			net_force;
	vec3			net_torque;
	vec3			old_position;
	matrix3			old_orientation;

	vec3 wishdir;


	train_t train;

	func_path_t path;
	pid_state_t pid;
};

#endif
