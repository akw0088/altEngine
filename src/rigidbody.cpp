#include "rigidbody.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ACCEL (0.25f)
#define AIR_ACCEL (0.5f)
#define MAX_SPEED 3.0f
#define MAX_AIR_SPEED 5.5f

#define DECELERATION_RATE 0.95f

RigidBody::RigidBody(Entity *entity)
: Model(entity)
{
	flags.sleep = false;
	flags.gravity = true;
	flags.noclip = false;
	flags.pursue_flag = true;
	flags.step_flag = false;
	flags.water = false;
	flags.flight = false;
	flags.last_water = false;
	flags.hard_impact = false;
	flags.lava = false;
	flags.slime = false;
	flags.rotational_friction_flag = false;
	flags.translational_friction_flag = false;
	flags.ground_friction_flag = false;
	flags.on_ground = false;
	y_offset = 0;
	target = NULL;
	mass = 10.0f;
	sphere_target = vec3(0.0f, 0.0f, 0.0f);
	jump_timer = 0; // move this to Player
	bounce = 0; // number of impacts
	step_type = 0;
	impact_velocity = 0.0f;
	water_depth = 0.0f;
	bsp_trigger_volume = 0;
	bsp_model_platform = 0;

	restitution = 0.5f; // boxes should never rest
	float height = 10.0f / UNITS_TO_METERS;
	float width = 10.0f / UNITS_TO_METERS;
	float depth = 10.0f / UNITS_TO_METERS;

	rotational_friction = 0.99f;
	translational_friction = 0.99f;


	train.escort = true;
	train.speed = 8.5f;
	train.wait = 100;
	train.path_min_dist = 75.0f;

	world_tensor.m[0] = 0.0f;
	world_tensor.m[1] = 0.0f;
	world_tensor.m[2] = 0.0f;
	world_tensor.m[3] = 0.0f;
	world_tensor.m[4] = 0.0f;
	world_tensor.m[5] = 0.0f;
	world_tensor.m[6] = 0.0f;
	world_tensor.m[7] = 0.0f;
	world_tensor.m[8] = 0.0f;

	inverse_tensor.m[0] = 12.0f / (mass *  (height * height + depth * depth));
	inverse_tensor.m[1] = 0;
	inverse_tensor.m[2] = 0;

	inverse_tensor.m[3] = 0;
	inverse_tensor.m[4] = 12.0f / (mass *  (width * width + depth * depth));
	inverse_tensor.m[5] = 0;
	
	inverse_tensor.m[6] = 0;
	inverse_tensor.m[7] = 0;
	inverse_tensor.m[8] = 12.0f / (mass *  (width * width + height * height));


	memset(&path, 0, sizeof(func_path_t));
	path.num_path = 0;
	path.target = &path.path_list[0];
	path.next = &path.path_list[1];

	init_pid(&pid);




	#define SND_GRENADE_IMPACT 244
	impact_index = SND_GRENADE_IMPACT;
}

void RigidBody::recalc()
{
	float height = (aabb[7].y - aabb[0].y);
	float width = (aabb[7].x - aabb[0].x);
	float depth = (aabb[7].z - aabb[0].z);
	mass = (height * width * depth) / 800.0f;
	if (abs32(mass) <= 0.00001f)
		mass = 10.0f;

	inverse_tensor.m[0] = 1.0f / (mass *  (height * height + depth * depth));
	inverse_tensor.m[1] = 0;
	inverse_tensor.m[2] = 0;

	inverse_tensor.m[3] = 0;
	inverse_tensor.m[4] = 1.0f / (mass *  (width * width + depth * depth));
	inverse_tensor.m[5] = 0;
	
	inverse_tensor.m[6] = 0;
	inverse_tensor.m[7] = 0;
	inverse_tensor.m[8] = 1.0f / (mass *  (width * width + height * height));
}

// Quake bunny hopping
void PM_Accelerate(const vec3 &wishdir, float wishspeed, float accel, Entity *ent)
{
	float delta_time = 0.08f;
	vec3 vel = ent->rigid->velocity;
	vel.y = 0.0f;

	// q2 style
	float		addspeed, accelspeed, currentspeed;

	currentspeed = vel  * wishdir;
	addspeed = wishspeed - currentspeed;
	if (addspeed <= 0)
	{
		return;
	}
	accelspeed = accel * delta_time * wishspeed;
	if (accelspeed > addspeed)
	{
		accelspeed = addspeed;
	}


	ent->rigid->velocity.x = wishdir.x * accelspeed;
	ent->rigid->velocity.z = wishdir.z * accelspeed;
}

/*
	Integrate physical quantaties over time by a fixed time step

	Writes to acceleration, angular_acceleration, angular velocity, velocity, position,
	and morientation
*/
void RigidBody::integrate(float time)
{
	matrix3 rotation;
	vec3 acceleration, angular_acceleration;

	if (flags.sleep)
		return;

	//translational


	acceleration = net_force / mass;
	if (flags.gravity == true && flags.noclip == false && flags.flight == false)
	{
		acceleration.y -= GRAVITY * GRAVITY_SCALE;
	}

	velocity = velocity + acceleration * time;

	// This is a really large cap for hopefully impossible situations
	if (velocity.magnitude() > MAX_VELOCITY)
		velocity *= DECELERATION_RATE;// velocity.normalize() * MAX_VELOCITY;

	/*
	if (entity->player)
	{
		velocity.x = 0.0f;
		velocity.z = 0.0f;

		PM_Accelerate(wishdir, 10.0f, 5.0f, entity);
	}
	else
	{
		*/
	// Clamp velocity on Y much lower
	// (jumppads / falling can integrate through solid objects if going too fast)
	if (velocity.y > 10.0f)
		velocity.y = 10.0f;
	if (velocity.y < -10.0f)
		velocity.y = -10.0f;


	old_position = entity->position;
	entity->position = entity->position + velocity * time * UNITS_TO_METERS
		+ acceleration * time * time * 0.5 * UNITS_TO_METERS;
	// Went ahead and used "simplified velocity verlet" as it subjectively feels nicer
	// Although it looks a lot like constant acceleration motion equations: x = x0 + vt + 0.5at^2
//	}


	//rotational
	angular_acceleration = world_tensor * net_torque;
	angular_velocity = angular_velocity + angular_acceleration * time;

	if (flags.translational_friction_flag || flags.noclip == true || flags.flight == true)
	{
		velocity *= translational_friction; // added translational "friction"
	}

	if (flags.ground_friction_flag && flags.on_ground)
	{
		velocity *= translational_friction; // added translational "friction"
	}

	if (flags.rotational_friction_flag || flags.water == true || flags.noclip == true)
	{
		angular_velocity *= rotational_friction; // added rotational "friction"
	}
	rotation.star(angular_velocity);

	old_orientation = morientation;
	morientation = morientation + morientation * rotation * time; 
	morientation.normalize();
	world_tensor = morientation * inverse_tensor * morientation.transpose();
}




/*
	Detects a collision with a plane and applies physical impulse response
*/
bool RigidBody::collision_detect(Plane &p)
{
	for( int i = 0;	i < 8; i++)
	{
		// make center origin
		vec3 point = center + aabb[i];

		//rotate around origin
		point = morientation * point;

		// rotate center around about true origin
		vec3 offset = morientation * center;

		// translate back to local coordinate origin
		point = point - offset;

		// translate to world coordinates
		point = point + entity->position;

		float d = point * p.normal + p.d;

		if ( d < -0.25f )
		{
			// Simulated too far
			return true;
		}
		else if ( d < 0.0f )
		{
			// Colliding
			// To handle multiple collision we need to store temp velocities

			// convert rotated point back to local coordinates
			point = point - entity->position;

			// convert point back to radius from center
			point = point + offset;

			// convert to meters
			point = point * (1.0f / UNITS_TO_METERS);

			// apply impulse to plane and radius vector
			impulse(p, point);
			entity->position = old_position;
			morientation = old_orientation;
		}
	}
	return false;
}

/*
	Applys collision impulse to a vertex
	radius must be in units of meters from CM
*/
void RigidBody::impulse(Plane &plane, vec3 &radius)
{
	float	impulse_numerator;
	float	impulse_denominator;
	vec3	impulse_momentum;

	vec3	vertex_velocity = velocity + vec3::crossproduct(angular_velocity, radius);

	// coefficient of resistution * -relative velocity
	impulse_numerator = -(1.0f + restitution) * (vertex_velocity * plane.normal);

	// 1/mass + N dot [((1/I)(radius cross normal)) cross radius] -- units of momentum p = mv
	impulse_denominator = (1.0f / mass) + plane.normal *
		vec3::crossproduct(world_tensor * vec3::crossproduct(radius, plane.normal), radius);
    
    impulse_momentum = plane.normal * (impulse_numerator/impulse_denominator);

	// apply impulse to primary quantities
    velocity += impulse_momentum / mass;
    angular_velocity = world_tensor * vec3::crossproduct(radius, -impulse_momentum);
}

void RigidBody::impulse(RigidBody &rigid, vec3 &point)
{
	float	impulse_numerator;
	float	impulse_denominator;
	vec3	impulse_force;
	vec3	local_point = (point - entity->position) * (1.0f / UNITS_TO_METERS);

	vec3	relative_velocity = (velocity - rigid.velocity) + vec3::crossproduct(angular_velocity, local_point) - vec3::crossproduct(rigid.angular_velocity, local_point);
	vec3	normal = rigid.entity->position - point;
	normal.normalize();

	// coefficient of resistution * -relative velocity
	impulse_numerator = -(1.0f + restitution) * (relative_velocity * normal);

	// 1/mass + N dot [((1/I)(radius cross normal)) cross radius] -- units of momentum p = mv
	impulse_denominator = (1.0f / mass) + (1.0f / rigid.mass)
		+ normal * vec3::crossproduct(world_tensor * vec3::crossproduct(local_point, normal), local_point)
		+ normal * vec3::crossproduct(rigid.world_tensor * vec3::crossproduct(local_point, normal), local_point);
    
    impulse_force = normal * (impulse_numerator/impulse_denominator);

	// apply impulse to primary quantities
    velocity += impulse_force * (1.0f / mass);
    angular_velocity = world_tensor * vec3::crossproduct(local_point, -impulse_force);
	rigid.velocity -= impulse_force * (1.0f / rigid.mass);
	rigid.angular_velocity = rigid.world_tensor * vec3::crossproduct(local_point, impulse_force);
	flags.sleep = false;
	rigid.flags.sleep = false;
	flags.gravity = true;
}

void RigidBody::impulse(RigidBody &rigid, vec3 &point, Plane &plane)
{
	float	impulse_numerator;
	float	impulse_denominator;
	vec3	impulse_force;
	vec3	local_point = (point - entity->position);

	vec3	relative_velocity = (velocity - rigid.velocity) + vec3::crossproduct(angular_velocity, local_point) - vec3::crossproduct(rigid.angular_velocity, local_point);
	vec3	normal = plane.normal;
	normal.normalize();

	// coefficient of resistution * -relative velocity
	impulse_numerator = -(1.0f + restitution) * (relative_velocity * normal);

	// 1/mass + N dot [((1/I)(radius cross normal)) cross radius] -- units of momentum p = mv
	impulse_denominator = (1.0f / mass) + (1.0f / rigid.mass)
		+ normal * vec3::crossproduct(world_tensor * vec3::crossproduct(local_point, normal), local_point)
		+ normal * vec3::crossproduct(rigid.world_tensor * vec3::crossproduct(local_point, normal), local_point);
    
	impulse_force = normal * (impulse_numerator/impulse_denominator);

	// apply impulse to primary quantities
	velocity += impulse_force * (1.0f / mass);
	angular_velocity = world_tensor * vec3::crossproduct(local_point, -impulse_force);
	rigid.velocity -= impulse_force * (1.0f / rigid.mass);
	rigid.angular_velocity = rigid.world_tensor * vec3::crossproduct(local_point, impulse_force);
	flags.sleep = false;
	rigid.flags.sleep = false;
	flags.gravity = true;
}


/*
	Detects a collision between current entity and a point
*/
bool RigidBody::collision_detect(vec3 &v)
{
	vec3 a, b;

	a = aabb[0] + entity->position + center;
	b = aabb[7] + entity->position + center;

	if ( (v.x > a.x) && (v.x < b.x) )
	{
		if ( (v.y > a.y) && (v.y < b.y) )
		{
			if ( (v.z > a.z) && (v.z < b.z) )
			{
				return true;
			}
		}
	}
	return false;
}

/*
	Detects a collision between current entity and another
*/
bool RigidBody::collision_detect_simple(RigidBody &body)
{
	for( int i = 0;	i < 8; i++)
	{
		vec3 point = body.aabb[i] + body.center;
		point = (body.morientation * point) + body.entity->position + body.center;
		if ( collision_detect(point) )
		{
			impulse(body, point);
			entity->position = old_position;
			morientation = old_orientation;
			return true;
		}
	}
	return false;
}

bool RigidBody::collision_distance(RigidBody &body)
{
	vec3 distance = body.entity->position - entity->position;

	// Dead players wont collide
	if (body.entity->player && body.entity->player->health < 0)
		return false;

	if (entity->player && entity->player->health < 0)
		return false;


	// two bots wont collide
	if (body.entity->player && entity->player)
		return false;

	if (distance.magnitude() < 40.0)
	{
		if (distance.magnitude() < 10.0f)
		{
			//telefrag
			if (body.entity->player && body.entity->player->health > 0)
			{
				body.entity->player->telefragged = entity;
			}
			else if (entity->player && entity->player->health > 0)
			{
				entity->player->telefragged = body.entity;
			}
		}
		// add some seperating force
		velocity += -distance.normalize() * (40.0f / distance.magnitude());

		return true;
	}
	else
	{
		return false;
	}
}

bool RigidBody::collision_detect(RigidBody &body)
{
	Plane plane[6];
	vec3 point;

	// Bounding box planes
	plane[0] = vec4(0.0f, 1.0f, 0.0f, -0.5f); // up
	plane[1] = vec4(0.0f, -1.0f, 0.0f, 0.5f); // down
	plane[2] = vec4(1.0f, 0.0f, 0.0f, -0.5f); // right
	plane[3] = vec4(-1.0f, 0.0f, 0.0f, 0.5f); // left
	plane[4] = vec4(0.0f, 0.0f, 1.0f, -0.5f); // far
	plane[5] = vec4(0.0f, 0.0f, -1.0f, 0.5f); // near


	// Rotate and translate planes to world space
	for(int i = 0; i < 6; i++)
	{
		point = plane[i].normal * 0.5f;						// point on plane
		point = morientation * point + entity->position;				// rotate point
		plane[i].normal = morientation * plane[i].normal;	// rotate normal
		plane[i].normal.normalize();
		plane[i].d = -(plane[i].normal * point);			// recalculate D
	}

	// Check if point of body is inside our bounding box
	for(int i = 0; i < 8; i++)
	{
		const float padding = 0.0125f;
		float depth = 1000.0f;
		int count = 0;
		int closest = 0;

		// oriented body point in world space
		point = body.morientation * body.aabb[i] + body.entity->position;

		for(int j = 0; j < 6; j++)
		{
			float distance = plane[j].normal * point + plane[j].d;
			if ( distance < padding)
			{
				if (depth > abs32(distance))
				{
					depth = abs32(distance);
					closest = j;
				}
				count++;
			}
		}

		// We had a point inside all 6 planes
		if (count == 6)
		{
			// Too deep, divide time step
			if (depth > 0.125f)
				return true;

			printf("body body impact\n");
			impulse(body, point, plane[closest]);
			entity->position = old_position;
			morientation = old_orientation;
		}
	}
	return false;
}

void RigidBody::frame2ent(Frame *camera, input_t &input)
{
	vec3		forward = camera->forward;
	vec3		up(0.0f, 1.0f, 0.0f);
	vec3		right;

//	forward.y = 0.0f;
//	forward.normalize();
	right = vec3::crossproduct(forward, up);
	right.normalize();

	if (entity->rigid)
	{
		//		entity->rigid->sleep = false;
		entity->rigid->flags.gravity = true;

		camera->pos = entity->position + vec3(0.0f, (float)y_offset, 0.0f);

		morientation.m[0] = right.x;
		morientation.m[1] = right.y;
		morientation.m[2] = right.z;

		morientation.m[3] = camera->up.x;
		morientation.m[4] = camera->up.y;
		morientation.m[5] = camera->up.z;

		morientation.m[6] = -forward.x;
		morientation.m[7] = -forward.y;
		morientation.m[8] = -forward.z;
	}

}

void RigidBody::frame2ent_yaw(Frame *camera, input_t &input)
{
	vec3		forward = camera->forward;
	vec3		up(0.0f, 1.0f, 0.0f);
	vec3		right;

	forward.y = 0.0f;
	forward.normalize();

	right = vec3::crossproduct(forward, up);
	right.normalize();
	up = vec3::crossproduct(right, forward);
	up.normalize();

	//	forward.y = 0.0f;
	//	forward.normalize();
	right = vec3::crossproduct(forward, up);
	right.normalize();

	//entity->rigid->sleep = false;
	entity->rigid->flags.gravity = true;
	camera->pos = entity->position;

	morientation.m[0] = right.x;
	morientation.m[1] = right.y;
	morientation.m[2] = right.z;

	morientation.m[3] = camera->up.x;
	morientation.m[4] = camera->up.y;
	morientation.m[5] = camera->up.z;

	morientation.m[6] = -forward.x;
	morientation.m[7] = -forward.y;
	morientation.m[8] = -forward.z;
}

void RigidBody::save_config(cfg_t &config)
{
	config.morientation = morientation;
	config.world_tensor = world_tensor;
	config.velocity = velocity;
	config.angular_velocity = angular_velocity;
	config.position = entity->position;
}

void RigidBody::load_config(cfg_t &config)
{
	morientation = config.morientation;
	world_tensor = config.world_tensor;
	velocity = config.velocity;
	angular_velocity = config.angular_velocity;
	entity->position = config.position;
}


void RigidBody::seek(const vec3 &position)
{
	vec3 direction = position - entity->position;
	direction.normalize();
	direction *= 8.0f; // speed
	net_force += direction - velocity;
}

void RigidBody::flee(const vec3 &position)
{
	vec3 direction = entity->position - position;
	direction.normalize();
	direction *= 8.0f; // speed
	net_force += direction - velocity;
}

//not quite right yet
void RigidBody::arrive(const vec3 &position)
{
	vec3 direction = position - entity->position;
	float distance = direction.magnitude();
	float speed = distance / 5.0f;
	if (speed > 8.0f)
		speed = 8.0f;
	direction = (direction / distance) * speed;
	net_force += direction - velocity;


	// visual doesnt affect above at all
	direction.y = 0.0f;
	vec3 up(0.0f, 1.0f, 0.0f);
	vec3 right = vec3::crossproduct(up, direction);
	right.normalize();
	morientation.m[0] = right.x;
	morientation.m[1] = right.y;
	morientation.m[2] = right.z;

	morientation.m[6] = direction.x;
	morientation.m[7] = 0.0f;
	morientation.m[8] = direction.z;
}

void RigidBody::pursue()
{
	vec3 direction = target->position - entity->position;
	float predict_time = direction.magnitude() / ( 8.0f + 8.0f );
	vec3 position = target->position + target->rigid->velocity * predict_time;

	seek(position);
}

void RigidBody::evade()
{
	vec3 direction = target->position - entity->position;
	float predict_time = direction.magnitude() / (8.0f + 8.0f);
	vec3 position = target->position + target->rigid->velocity * predict_time;

	flee(position);
}

void RigidBody::set_target(Entity &ent)
{
	target = &ent;
}


void RigidBody::wander(float radius, float distance, float jitter)
{
	// make random point on sphere
	float x = ((rand() % 1000) / 1000.0f) * jitter;
	float y = ((rand() % 1000) / 1000.0f) * jitter;
	float z = ((rand() % 1000) / 1000.0f) * jitter;

	sphere_target += vec3(x, y, z);
	sphere_target = sphere_target.normalize() * radius;

	// Move sphere infront of us
	vec3 forward;

	// Right was forward coming out of the lightning gun for some reason
	// fix this ;)
	getForwardVector(forward);
	vec3 up(0.0f, 1.0f, 0.0f);
	vec3 right = vec3::crossproduct(up, forward);
	sphere_target += right * distance;

	// Move towards the random point on sphere
	seek(sphere_target + entity->position);
}

float RigidBody::get_volume()
{
	//water_density = 1000.0f; // kg/m3
	//Force_buoyant = volume * density of water * gravity // upward force of displaced water
	float width = abs32(aabb[0].x - aabb[7].x);
	float length = abs32(aabb[0].y - aabb[7].y);
	float height = abs32(aabb[0].z - aabb[7].z);

	return width * length * height;
}

float RigidBody::get_height()
{
	// max should always be bigger than min, but abs anyway
	return abs32(aabb[7].z - aabb[0].z);
}

float *RigidBody::get_matrix(float *matrix)
{
	matrix[0] = morientation.m[0];
	matrix[1] = morientation.m[1];
	matrix[2] = morientation.m[2];
	matrix[3] = 0.0f;

	matrix[4] = morientation.m[3];
	matrix[5] = morientation.m[4];
	matrix[6] = morientation.m[5];
	matrix[7] = 0.0f;

	matrix[8] = morientation.m[6];
	matrix[9] = morientation.m[7];
	matrix[10] = morientation.m[8];
	matrix[11] = 0.0f;

	/* matrix rotates around center, but position is arbitrary point
	from which verts are defined */

	vec3 offset;

	vec3 temp = center + vec3(0.0f, (float)-y_offset, 0.0f);
	offset = morientation * temp;

	matrix[12] = entity->position.x - offset.x;
	matrix[13] = entity->position.y - offset.y;
	matrix[14] = entity->position.z - offset.z;
	matrix[15] = 1.0f;
	return matrix;
}

void RigidBody::get_frame(Frame &frame)
{
	Model::get_frame(frame);

	frame.pos += vec3(0.0f, (float)y_offset, 0.0f);
}

bool RigidBody::flight_move(input_t &input, float speed_scale)
{
	Frame camera;
	Frame yaw;

	wishdir = vec3();

	get_frame(camera);
	vec3	forward = camera.forward;
	vec3	right = vec3::crossproduct(camera.up, camera.forward);
	bool	moved = false;
	bool	ret = false;


	vec3 yaw_right;
	yaw.up = vec3(0.0f, 1.0f, 0.0f);
	yaw.forward = camera.forward;
	yaw.forward.y = 0.0f;
	yaw.forward.normalize();

	yaw_right = vec3::crossproduct(yaw.up, yaw.forward);
	yaw_right.normalize();
	yaw.up = vec3::crossproduct(right, yaw.forward);
	yaw.up.normalize();

	if (y_offset != 0 || input.walk)
	{
		speed_scale *= 0.5f;
	}

	if (jump_timer > 0)
		jump_timer--;

	flags.sleep = false;

	if (input.moveup)
	{
		wishdir += -forward;
		moved = true;
	}

	if (input.movedown)
	{
		wishdir += forward;
		moved = true;
	}

	if (input.moveleft)
	{
		wishdir += -right;
		moved = true;
	}

	if (input.moveright)
	{
		wishdir += right;
		moved = true;
	}

	if (input.jump)
	{
		wishdir += vec3(0.0f, 1.0f, 0.0f);
		velocity.y += entity->player->accel * speed_scale;
		moved = true;
	}

	if (input.duck)
	{
		wishdir += vec3(0.0f, -1.0f, 0.0f);
		velocity.y += -entity->player->accel * speed_scale;
		moved = true;
	}

	//	wishdir = wishdir.normalize();


	if (moved)
	{
		velocity += wishdir * entity->player->accel * speed_scale;
	}
	float speed = 0.0f;
	static bool hopped = false;

	if ((hopped || wishdir.magnitude() > 1.0f))
	{
		speed = newtonSqrt(velocity.x * velocity.x + velocity.z * velocity.z) - (0.2f * entity->player->max_speed * speed_scale);
		hopped = true;
	}
	else
	{
		speed = newtonSqrt(velocity.x * velocity.x + velocity.z * velocity.z) * speed_scale;
	}


	if (speed > entity->player->max_air_speed * speed_scale)
	{
		//		printf("MAX_AIR_SPEEDED\n");
		velocity.x *= (entity->player->max_air_speed * speed_scale / speed);
		//		velocity.y *= (entity->player->max_air_speed * speed_scale / speed);
		velocity.z *= (entity->player->max_air_speed * speed_scale / speed);
	}


	if (moved == false)
	{
		// deceleration
		//		printf("FRICTIONED\n");
		velocity.x *= 0.5f;
		velocity.z *= 0.5f;
	}

	return ret;
}

bool RigidBody::water_move(input_t &input, float speed_scale)
{
	Frame camera;
	Frame yaw;

	wishdir = vec3();

	get_frame(camera);
	vec3	forward = camera.forward;
	vec3	right = vec3::crossproduct(camera.up, camera.forward);
	bool	moved = false;
	bool	ret = false;


	vec3 yaw_right;
	yaw.up = vec3(0.0f, 1.0f, 0.0f);
	yaw.forward = camera.forward;
	yaw.forward.y = 0.0f;
	yaw.forward.normalize();

	yaw_right = vec3::crossproduct(yaw.up, yaw.forward);
	yaw_right.normalize();
	yaw.up = vec3::crossproduct(right, yaw.forward);
	yaw.up.normalize();

	if (y_offset != 0 || input.walk)
	{
		speed_scale *= 0.5f;
	}

	if (jump_timer > 0)
		jump_timer--;

	flags.sleep = false;

	if (input.moveup)
	{
		wishdir += -forward;
		moved = true;
	}

	if (input.movedown)
	{
		wishdir += forward;
		moved = true;
	}

	if (input.moveleft)
	{
		wishdir += -right;
		moved = true;
	}

	if (input.moveright)
	{
		wishdir += right;
		moved = true;
	}

	if (input.jump)
	{
//		wishdir += vec3(0.0f, 1.0f, 0.0f);
		velocity.y += entity->player->accel * speed_scale;
		moved = true;
	}
	if (input.duck)
	{
//		wishdir += vec3(0.0f, -1.0f, 0.0f);
		velocity.y += -entity->player->accel * speed_scale;
		moved = true;
	}

	if (moved)
	{
		velocity += wishdir * entity->player->accel * speed_scale * 0.5f;
	}
	float speed = 0.0f;



	if (speed > entity->player->max_air_speed * 0.5f * speed_scale)
	{
		//		printf("MAX_AIR_SPEEDED\n");
		velocity.x *= (entity->player->max_air_speed * 0.5f * speed_scale / speed);
//		velocity.y *= (entity->player->max_air_speed * 0.5f * speed_scale / speed);
		velocity.z *= (entity->player->max_air_speed * 0.5f * speed_scale / speed);
	}


	if (moved == false)
	{
		// deceleration
		//		printf("FRICTIONED\n");
		velocity.x *= 0.5f;
		velocity.y *= 0.5f;
		velocity.z *= 0.5f;
	}


	return ret;
}

bool RigidBody::air_move(input_t &input, float speed_scale)
{
	float air_control = 1.0f;
	Frame camera;
	Frame yaw;

	wishdir = vec3();

	air_control = entity->player->air_control;

	get_frame(camera);
	vec3	forward = camera.forward;
	vec3	right = vec3::crossproduct(camera.up, camera.forward);
	bool	moved = false;
	bool	ret = false;


	vec3 yaw_right;
	yaw.up = vec3(0.0f, 1.0f, 0.0f);
	yaw.forward = camera.forward;
	yaw.forward.y = 0.0f;
	yaw.forward.normalize();

	yaw_right = vec3::crossproduct(yaw.up, yaw.forward);
	yaw_right.normalize();
	yaw.up = vec3::crossproduct(right, yaw.forward);
	yaw.up.normalize();

	//prevent walking upward
	forward.y = 0.0f;
	right.y = 0.0f;

	if (jump_timer > 0)
		jump_timer--;

	flags.sleep = false;

	if (input.moveup)
	{
		wishdir += -forward;
		moved = true;
	}

	if (input.movedown)
	{
		wishdir += forward;
		moved = true;
	}

	if (input.moveleft)
	{
		wishdir += -right;
		moved = true;
	}

	if (input.moveright)
	{
		wishdir += right;
		moved = true;
	}

	if (input.duck)
	{
		wishdir += vec3(0.0f, -1.0f, 0.0f);
		moved = true;
	}

	if (moved)
	{
		velocity += wishdir * entity->player->air_accel * air_control * speed_scale;
	}
	float speed = 0.0f;
	static bool hopped = false;

	if (hopped || wishdir.magnitude() > 1.0f)
	{
		speed = newtonSqrt(velocity.x * velocity.x + velocity.z * velocity.z) - (0.2f * entity->player->max_speed * speed_scale);
		hopped = true;
	}
	else
	{
		speed = newtonSqrt(velocity.x * velocity.x + velocity.z * velocity.z) * speed_scale;
	}

	if (speed > entity->player->max_air_speed * speed_scale)
	{
		//		printf("MAX_AIR_SPEEDED\n");
		velocity.x *= (entity->player->max_air_speed * speed_scale / speed);
		//		velocity.y *= (entity->player->max_air_speed * speed_scale / speed);
		velocity.z *= (entity->player->max_air_speed * speed_scale / speed);
	}


	if (moved == false)
	{
		// deceleration
		//		printf("FRICTIONED\n");
		velocity.x *= 0.5f;
		velocity.z *= 0.5f;
	}

	return ret;
}

bool RigidBody::ground_move(input_t &input, float speed_scale)
{
	static int two_frames = 0;
	float jump_scale = 0.65f;
	Frame camera;
	Frame yaw;

	wishdir = vec3();

	entity->player->max_air_speed = MAX_AIR_SPEED * speed_scale;

	get_frame(camera);
	vec3	forward = camera.forward;
	vec3	right = vec3::crossproduct(camera.up, camera.forward);
	bool	moved = false;
	bool	jumped = false;
	bool	jumppad = false;
	bool	ret = false;


	vec3 yaw_right;
	yaw.up = vec3(0.0f, 1.0f, 0.0f);
	yaw.forward = camera.forward;
	yaw.forward.y = 0.0f;
	yaw.forward.normalize();

	yaw_right = vec3::crossproduct(yaw.up, yaw.forward);
	yaw_right.normalize();
	yaw.up = vec3::crossproduct(right, yaw.forward);
	yaw.up.normalize();

	if (y_offset != 0 || input.walk)
	{
		speed_scale *= 0.5f;
	}

	//prevent walking upward
	forward.y = 0.0f;
	right.y = 0.0f;

	if (jump_timer > 0)
		jump_timer--;

	flags.sleep = false;

	if (input.moveup)
	{
		wishdir += -forward;
		moved = true;
	}

	if (input.movedown)
	{
		wishdir += forward;
		moved = true;
	}

	if (input.moveleft)
	{
		wishdir += -right;
		moved = true;
	}

	if (input.moveright)
	{
		wishdir += right;
		moved = true;
	}

	if (input.jump)
	{
		jumped = true;
		moved = true;
	}
	if (input.duck)
	{
		wishdir += vec3(0.0f, -1.0f, 0.0f);
		moved = true;
	}

	//	wishdir = wishdir.normalize();


	if (moved)
	{
		velocity += wishdir * entity->player->accel * speed_scale;
	}
	float speed = 0.0f;

	speed = newtonSqrt(velocity.x * velocity.x + velocity.z * velocity.z) * speed_scale;

	if (entity->player && entity->player->jumppad_timer > 0)
	{
		jumppad = true;
	}


	if (flags.on_ground && (speed > entity->player->max_speed * speed_scale))
	{
		if (jumppad == false && two_frames > 8)
		{
			//			printf("MAX_SPEEDED\n");
			two_frames = 0;
			velocity.x *= (entity->player->max_speed * speed_scale / speed);
			//			velocity.y *= (MAX_SPEED * speed_scale / speed);
			velocity.z *= (entity->player->max_speed * speed_scale / speed);
		}
		two_frames++;
	}

	if (jumppad == true && (speed > entity->player->max_air_speed * speed_scale))
	{
		//		printf("MAX_AIR_SPEEDED\n");
		velocity.x *= (entity->player->max_air_speed * speed_scale / speed);
		velocity.z *= (entity->player->max_air_speed * speed_scale / speed);
	}


	if (moved)
	{
		if (jumped && jump_timer == 0)
		{
			velocity.y += 3.0f * jump_scale * GRAVITY_SCALE;
			velocity += wishdir * 1.25f;
			jump_timer = (int)(TICK_RATE * 0.3f);
			ret = true;
		}
	}
	else
	{
		// deceleration
		//		printf("FRICTIONED\n");
		velocity.x *= 0.5f;
		velocity.z *= 0.5f;
	}

	return ret;
}


bool RigidBody::move(input_t &input, float speed_scale)
{
	if (flags.noclip)
	{
		return flight_move(input, 5.0);
	}
	else if (flags.flight)
	{
		return flight_move(input, speed_scale);
	}
	else if (flags.water && water_depth < 2048.0f)
	{
		return water_move(input, speed_scale);
	}
	else if (flags.on_ground == false)
	{
		return air_move(input, speed_scale);
	}
	else
	{
		return ground_move(input, speed_scale);
	}
}

#if 0
bool RigidBody::move(input_t &input, float speed_scale)
{
	static int two_frames = 0;
	float air_control = 1.0f;
	float jump_scale = 0.65f;
	Frame camera;
	Frame yaw;

	wishdir = vec3();

	if (on_ground == false)
	{
		air_control = entity->player->air_control;
	}
	else
	{
		entity->player->max_air_speed = MAX_AIR_SPEED * speed_scale;
	}

	if (water || flight || noclip)
		air_control = 1.0f;


	get_frame(camera);
	vec3	forward = camera.forward;
	vec3	right = vec3::crossproduct(camera.up, camera.forward);
	bool	moved = false;
	bool	jumped = false;
	bool	jumppad = false;
	bool	ret = false;


	vec3 yaw_right;
	yaw.up = vec3(0.0f, 1.0f, 0.0f);
	yaw.forward = camera.forward;
	yaw.forward.y = 0.0f;
	yaw.forward.normalize();

	yaw_right = vec3::crossproduct(yaw.up, yaw.forward);
	yaw_right.normalize();
	yaw.up = vec3::crossproduct(right, yaw.forward);
	yaw.up.normalize();

	if (y_offset != 0 || input.walk)
	{
		speed_scale *= 0.5f;
	}

	if (noclip)
		speed_scale *= 1.5f;

	//prevent walking upward
	if (noclip == false && flight == false)
	{
		forward.y = 0.0f;
		right.y = 0.0f;
	}

	if (jump_timer > 0)
		jump_timer--;

	sleep = false;

	if (input.moveup)
	{
		wishdir += -forward;
		moved = true;
	}

	if (input.movedown)
	{
		wishdir += forward;
		moved = true;
	}

	if (input.moveleft)
	{
		wishdir += -right;
		moved = true;
	}

	if (input.moveright)
	{
		wishdir += right;
		moved = true;
	}

	if (input.jump)
	{
		if (flight || water || noclip)
		{
			velocity.y += entity->player->accel * speed_scale;
		}
		else
		{
			jumped = true;
		}

		moved = true;
	}
	if (input.duck)
	{
		wishdir += vec3(0.0f, -1.0f, 0.0f);


		if ((flight || water || noclip) && (on_ground == false || noclip == true))
		{
			velocity.y += -entity->player->accel * speed_scale;
		}
		moved = true;
	}

	//	wishdir = wishdir.normalize();


	if (moved)
	{
		if (water == false && noclip == false && flight == false)
		{
			if (on_ground)
				velocity += wishdir * entity->player->accel * speed_scale;
			else
				velocity += wishdir * entity->player->air_accel * air_control * speed_scale;
		}
		else
		{
			velocity += wishdir * entity->player->accel * speed_scale;
		}
	}
	float speed = 0.0f;
	static bool hopped = false;

	if ((hopped || wishdir.magnitude() > 1.0f) && on_ground == false)
	{
		speed = newtonSqrt(velocity.x * velocity.x + velocity.z * velocity.z) - (0.2f * entity->player->max_speed * speed_scale);
		hopped = true;
	}
	else
	{
		speed = newtonSqrt(velocity.x * velocity.x + velocity.z * velocity.z) * speed_scale;
	}

	if (entity->player && entity->player->jumppad_timer > 0)
	{
		jumppad = true;
	}


	if (on_ground && (speed > entity->player->max_speed * speed_scale))
	{

		if (jumppad == false && two_frames > 8)
		{
			//			printf("MAX_SPEEDED\n");
			two_frames = 0;
			velocity.x *= (entity->player->max_speed * speed_scale / speed);
			//			velocity.y *= (MAX_SPEED * speed_scale / speed);
			velocity.z *= (entity->player->max_speed * speed_scale / speed);
			hopped = false;
		}
		two_frames++;
	}

	if (on_ground == false)
	{
		two_frames = 0;
	}


	if ((on_ground == false || jumppad == true) && (speed > entity->player->max_air_speed * speed_scale))
	{
		//		printf("MAX_AIR_SPEEDED\n");
		velocity.x *= (entity->player->max_air_speed * speed_scale / speed);
		//		velocity.y *= (entity->player->max_air_speed * speed_scale / speed);
		velocity.z *= (entity->player->max_air_speed * speed_scale / speed);
	}


	if (moved)
	{
		if ((on_ground && jumped && jump_timer == 0) || (water && water_depth <= 5.0f && jumped))
		{
			velocity.y += 3.0f * jump_scale * GRAVITY_SCALE;
			velocity += wishdir * 1.25f;
			jump_timer = (int)(TICK_RATE * 0.3f);
			ret = true;
		}
	}
	else
	{
		// deceleration
		//		printf("FRICTIONED\n");
		velocity.x *= 0.5f;
		velocity.z *= 0.5f;
	}

	// Speed up water movement due to additional deceleration friction
	if ((water_depth >= 2.0f && water) || noclip)
	{
		velocity.x *= (1.25f * speed_scale / speed);
		velocity.y *= (1.25f * speed_scale / speed);
		velocity.z *= (1.25f * speed_scale / speed);
	}


	return ret;
}
#endif


void RigidBody::move_forward(float speed_scale)
{
	input_t input;

	memset(&input, 0, sizeof(input_t));
	input.moveup = true;
	move(input, speed_scale);
}

void RigidBody::move_backward(float speed_scale)
{
	input_t input;

	memset(&input, 0, sizeof(input_t));
	input.movedown = true;
	move(input, speed_scale);
}

void RigidBody::move_left(float speed_scale)
{
	input_t input;

	memset(&input, 0, sizeof(input_t));
	input.moveleft = true;
	move(input, speed_scale);
}

void RigidBody::move_right(float speed_scale)
{
	input_t input;

	memset(&input, 0, sizeof(input_t));
	input.moveright = true;
	move(input, speed_scale);
}

void RigidBody::move_up(float speed_scale)
{
	input_t input;

	memset(&input, 0, sizeof(input_t));
	input.jump = true;
	move(input, speed_scale);
}

void RigidBody::move_down(float speed_scale)
{
	input_t input;

	memset(&input, 0, sizeof(input_t));
	input.duck = true;
	move(input, speed_scale);
}

void RigidBody::lookat(vec3 &target)
{
	Frame frame;
	vec3 right;

	frame.up = vec3(0.0f, 1.0f, 0.0f);
	frame.forward = (entity->position - target).normalize();

	right = vec3::crossproduct(frame.forward, frame.up);
	right.normalize();
	frame.up = vec3::crossproduct(right, frame.forward);
	frame.up.normalize();

	frame.set(entity->model->morientation);
}

void RigidBody::lookat_yaw(vec3 &target)
{
	Frame frame;
	vec3 right;

	frame.up = vec3(0.0f, 1.0f, 0.0f);
	frame.forward = (entity->position - target);
	frame.forward.y = 0.0f;
	frame.forward.normalize();

	right = vec3::crossproduct(frame.forward, frame.up);
	right.normalize();
	frame.up = vec3::crossproduct(right, frame.forward);
	frame.up.normalize();

	frame.set(entity->model->morientation);
}


typedef struct
{
	vec3 dx;
	vec3 dv;
} Derivative;

vec3 rk4_acceleration(const vec3 &pos, const vec3 &vel, float t)
{
	float k = 10;
	float b = 1;

	vec3 a = pos * -k + -vel * b;
	return a;
}

Derivative rk4_evaluate(const vec3 &pos, const vec3 &vel, float t)
{
	Derivative output;
	output.dx = vel;
	output.dv = rk4_acceleration(pos, vel, t);
	return output;
}

Derivative rk4_evaluate(const vec3 &pos, const vec3 &vel, float t, float dt, const Derivative &d)
{
	vec3 npos;
	vec3 nvel;
	npos = pos + d.dx * dt;
	nvel = vel + d.dv * dt;
	Derivative output;
	output.dx = nvel;
	output.dv = rk4_acceleration(npos, nvel, t + dt);
	return output;
}

void rk4_integrate(vec3 &pos, vec3 &vel, float t, float dt)
{
	Derivative a = rk4_evaluate(pos, vel, t);
	Derivative b = rk4_evaluate(pos, vel, t, dt*0.5f, a);
	Derivative c = rk4_evaluate(pos, vel, t, dt*0.5f, b);
	Derivative d = rk4_evaluate(pos, vel, t, dt, c);

	vec3 dxdt = ((a.dx + (b.dx + c.dx) + d.dx) * 2.0f) * (1.0f / 6.0f);
	vec3 dvdt = ((a.dv + (b.dv + c.dv) + d.dv) * 2.0f) * (1.0f / 6.0f);

	pos += dxdt * dt;
	vel += dvdt * dt;
}

// This is just a basic follow the path setup for the pid controller, kills momentum to make it nicer for eventual func_train use
void RigidBody::pid_follow_path(vec3 *path_list, int num_path, float max_velocity, float distance, int wait)
{
	Entity *projectile = entity;

	if ((*path.target - projectile->position).magnitude() < distance)
	{
		if (path.count == wait + 1)
		{
			path.count = 0;
			path.target = path.next;
			path.index++;

			if ( path.loop && path.index >= num_path)
			{
				path.index = 0;
			}
			else if (path.index >= num_path)
			{
				path.index = num_path - 1;
			}
			path.next = &path_list[path.index];
		}

		// increase count at target position until we hit wait
		path.count++;
		projectile->rigid->velocity = projectile->rigid->velocity.normalize() * 0.01f;
	}

	update_pid(&projectile->rigid->pid, *path.target, projectile->position, projectile->rigid->net_force);
	// Could probably use steering behavior arrive / follow etc here too, but the PID is faster
	
//	pid_controller(*path.target, 0.16f, projectile->position, projectile->rigid->velocity, 300.0f);
	
	if (projectile->rigid->velocity.magnitude() > max_velocity)
	{
		projectile->rigid->velocity *= DECELERATION_RATE;
	}
}


int RigidBody::train_follow_path(vec3 *target, float max_velocity, float distance, int wait)
{
	Entity *projectile = entity;

	/*
	if (path.start)
	{
		path.start = 0;
		path.target = target;
	}
	*/

	if ((*path.target - projectile->position).magnitude() < distance)
	{
		if (path.count == wait)
		{
			path.count = 0;
			return 0;
		}
		path.count++;
		projectile->rigid->velocity = vec3(0.0f, 0.0f, 0.0f);
	}
	else
	{
		pid_controller(*path.target, 0.16f, projectile->position, projectile->rigid->velocity, 0);
		if (projectile->rigid->velocity.magnitude() > max_velocity)
		{
			projectile->rigid->velocity *= DECELERATION_RATE;
		}
	}
	return -1;
}
