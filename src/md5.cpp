#include "md5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

MD5::MD5()
{
	loaded = false;
	model = NULL;
}

MD5::~MD5()
{
	if (loaded == false)
	{
		return;
	}
	delete[] model->joint;
	for (int i = 0; i < model->num_mesh; i++)
	{
		delete[] model->mesh[i].triangle;
		delete[] model->mesh[i].vertex;
		delete[] model->mesh[i].weight;
	}
	delete[] model->mesh;
	delete model;

	while (plist_stack.size())
	{
		delete plist_stack.top();
		plist_stack.pop();
	}
}

void MD5::InterpolateSkeletons(const md5_joint_t *skelA, const md5_joint_t *skelB, int num_joints, float interp, md5_joint_t *out)
{
	int i;

	for (i = 0; i < num_joints; ++i)
	{
		// Copy parent index
		out[i].parent = skelA[i].parent;

		// Linear interpolation for position
		out[i].pos = skelA[i].pos + (skelB[i].pos - skelA[i].pos) * interp;

		// Spherical linear interpolation for orientation
		quaternion::slerp(skelA[i].orient, skelB[i].orient, interp, out[i].orient);
	}
}

void MD5::PrepareMesh(int mesh_index, md5_joint_t *skeleton, int &num_index, int *index_array, vertex_t *vertex_array, int &num_vertex)
{
	md5_mesh_t *mesh = &model->mesh[mesh_index];
	int i, j, k = 0;
	

	// create index array
	num_index = 3 * mesh->num_tri;
	for (i = 0; i < mesh->num_tri; i++)
	{
		index_array[k++] = mesh->triangle[i].a;
		index_array[k++] = mesh->triangle[i].b;
		index_array[k++] = mesh->triangle[i].c;
	}
	num_vertex = mesh->num_vertex;

	// create vertex array
	for (i = 0; i < mesh->num_vertex; ++i)
	{
		vec3 position( 0.0f, 0.0f, 0.0f );
		vec3 normal = MD5::vertex_array[mesh_index][i].normal;
		vec3 tangent = MD5::vertex_array[mesh_index][i].tangent;

		// Calculate final vertex to draw with weights
		for (j = 0; j < mesh->vertex[i].count; ++j)
		{
			const md5_weight_t *weight = &mesh->weight[mesh->vertex[i].start + j];
			const md5_joint_t *joint = &skeleton[weight->joint];

			// Calculate transformed vertex for this weight
			vec3 weighted_vertex;
			quaternion q(0, weight->pos);
			q = (joint->orient * q) * joint->orient.conjugate();
			weighted_vertex.x = q.x;
			weighted_vertex.y = q.y;
			weighted_vertex.z = q.z;

			//normal and tangent
			q.x = normal.x;
			q.y = normal.y;
			q.z = normal.z;
			q = (joint->orient * q) * joint->orient.conjugate();
			normal.x = q.x;
			normal.y = q.y;
			normal.z = q.z;

			q.x = tangent.x;
			q.y = tangent.y;
			q.z = tangent.z;
			q = (joint->orient * q) * joint->orient.conjugate();
			tangent.x = q.x;
			tangent.y = q.y;
			tangent.z = q.z;

			// The sum of all weight->bias should be 1.0
			position += (joint->pos + weighted_vertex) * weight->bias;
		}

		vertex_array[i].position.x = position.x;
		vertex_array[i].position.y = position.z;
		vertex_array[i].position.z = -position.y;

		vertex_array[i].normal = normal;
		vertex_array[i].tangent = vec4(tangent.x, tangent.y, tangent.z, 1.0f);

		vertex_array[i].texCoord0.x = mesh->vertex[i].u;
		vertex_array[i].texCoord0.y = -mesh->vertex[i].v;
	}
}


int MD5::load_md5(char *file)
{
	char *data = get_file(file, NULL);
	char *pdata;
	char *pjoint;
	char *pmesh;

	int version;
	int num_joint;
	int num_mesh;

	md5_joint_t	*joint;
	md5_mesh_t	*mesh;
	int i;

	if (data == NULL)
	{
		debugf("failed to load md5 file %s", file);
		return -1;
	}

	pdata = strstr(data, "MD5Version");
	sscanf(pdata, "MD5Version %d", &version);

	pdata = strstr(data, "numJoints");
	sscanf(pdata, "numJoints %d", &num_joint);

	pdata = strstr(data, "numMeshes");
	sscanf(pdata, "numMeshes %d", &num_mesh);

	pjoint = strstr(data, "joints {");
	pmesh = strstr(data, "mesh {");


	joint = new md5_joint_t [num_joint];
	if (joint == NULL)
	{
		perror("malloc() failed");
		return -1;
	}

	mesh = new md5_mesh_t [num_mesh];
	if (mesh == NULL)
	{
		perror("malloc() failed");
		delete [] joint;
		return -1;
	}


	if ( parse_joint(pjoint, joint, num_joint) )
	{
		delete [] joint;
		delete [] mesh;
		return -1;
	}

	for(i = 0; i < num_mesh; i++)
	{
		if ( parse_mesh(pmesh, &mesh[i]) )
		{
			delete [] joint;
			delete [] mesh;
			return -1;
		}

		pmesh = strstr(pmesh + 6, "mesh {");
		if (pmesh == NULL && i + 1 < num_mesh)
		{
			debugf("Error: Unexpected end of mesh data\n");
			delete [] joint;
			delete [] mesh;
			return -1;
		}
	}
	model = new md5_model_t;
	memset(model, 0, sizeof(md5_model_t));
	strcpy(model->name, file);
	model->joint = joint;
	model->mesh = mesh;
	model->num_joint = num_joint;
	model->num_mesh = num_mesh;
	delete [] data;
	loaded = true;
	return 0;
}

int MD5::parse_joint(char *data, md5_joint_t *joint, int num_joint)
{
	char *pdata;
	char name[256];
	int parent;
	float x, y, z;
	float qx, qy, qz;

	int i = 0;

	pdata = strchr(data, '\"');
	while (pdata)
	{
		if ( 8 == sscanf(pdata, "%256s %d ( %f %f %f ) ( %f %f %f )", name, &parent, &x, &y, &z, &qx, &qy, &qz) )
		{
			strcpy(joint[i].name, name);
			joint[i].parent = parent;
			joint[i].pos.x = x;
			joint[i].pos.y = y;
			joint[i].pos.z = z;
			joint[i].orient.x = qx;
			joint[i].orient.y = qy;
			joint[i].orient.z = qz;
			joint[i].orient.compute_w();
			i++;
		}
		else
		{
			debugf("Error: Couldnt read joint %d\n", i);
			return 1;
		}

		if (i == num_joint)
			break;

		pdata = strchr(pdata, '\"'); // ending quote of name
		pdata = strchr(pdata, '\"'); // begining quote of new joint
	}
	return 0;
}

int MD5::parse_mesh(char *data, md5_mesh_t *mesh)
{
	char *pdata;
	int num_vertex;
	int num_tri;
	int num_weight;
	md5_vertex_t	*vertex;
	md5_triangle_t	*triangle;
	md5_weight_t	*weight;
	char shader[256] = {0};
	int i;


	pdata = strstr(data, "shader");
	sscanf(pdata, "shader %256s", shader);

	// Read in counts
	pdata = strstr(data, "numverts");
	sscanf(pdata, "numverts %d", &num_vertex);

	pdata = strstr(data, "numtris");
	sscanf(pdata, "numtris %d", &num_tri);

	pdata = strstr(data, "numweights");
	sscanf(pdata, "numweights %d", &num_weight);

	// Allocate memory
	vertex = new md5_vertex_t [num_vertex];
	if (vertex == NULL)
	{
		perror("malloc failed");
		return 1;
	}

	triangle = new md5_triangle_t [num_tri];
	if (triangle == NULL)
	{
		perror("malloc failed");
		delete [] vertex;
		return 1;
	}

	weight = new md5_weight_t [num_weight];
	if (weight == NULL)
	{
		perror("malloc failed");
		delete [] vertex;
		delete [] triangle;
		return 1;
	}


	// parse verts
	pdata = strstr(data, "vert 0");

	for( i = 0; i < num_vertex; i++)
	{
		int index;
		int start;
		int count;
		float u;
		float v;

		if (pdata == NULL)
		{
			debugf("Error: unexpected end of vert data\n");
			delete [] vertex;
			delete [] triangle;
			delete [] weight;
			return 1;
		}

		//vert 0 ( 0.0049510002 0.0049510002 ) 0 1
		if ( 5 == sscanf(pdata, "vert %d ( %f %f ) %d %d", &index, &u, &v, &start, &count) )
		{
			if (index != i)
			{
				debugf("Error: vertex not in expected order\n");
				delete [] vertex;
				delete [] triangle;
				delete [] weight;
				return 1;
			}

			vertex[i].u = u;
			vertex[i].v = v;
			vertex[i].start = start;
			vertex[i].count = count;
		}
		else
		{
			debugf("Error reading vertex %d\n", i);
			delete [] vertex;
			delete [] triangle;
			delete [] weight;
			return 1;
		}
		pdata = strstr(pdata + 5, "vert ");
	}

	// Parse triangles
	pdata = strstr(data, "tri 0");

	for (i = 0; i < num_tri; i++)
	{
		int index;
		int a, b, c;

		if (pdata == NULL)
		{
			debugf("Error: unexpected end of tri data\n");
			delete [] vertex;
			delete [] triangle;
			delete [] weight;
			return 1;
		}

		//tri 0 2 1 0
		if ( 4 == sscanf(pdata, "tri %d %d %d %d", &index, &a, &b, &c) )
		{
			if (index != i)
			{
				debugf("Error: triangle not in expected order\n");
				delete [] vertex;
				delete [] triangle;
				delete [] weight;
				return 1;
			}

			triangle[i].a = a;
			triangle[i].b = b;
			triangle[i].c = c;
		}
		else
		{
			debugf("Error reading triangle %d\n", i);
			delete [] vertex;
			delete [] triangle;
			delete [] weight;
			return 1;
		}
		pdata = strstr(pdata + 4, "tri ");
	}


	pdata = strstr(data, "weight 0");

	// Parse weights
	for (i = 0; i < num_weight; i++)
	{
		int index;
		int joint;
		float bias;
		float x, y, z;

		if (pdata == NULL)
		{
			debugf("Error: unexpected end of weight data\n");
			delete [] vertex;
			delete [] triangle;
			delete [] weight;
			return 1;
		}

		//weight 0 48 1 ( -1.3141413927 3.7706668377 4.5340304375 )
		if ( 6 == sscanf(pdata, "weight %d %d %f ( %f %f %f )", &index, &joint, &bias, &x, &y, &z) )
		{
			if (index != i)
			{
				debugf("Error: weight not in expected order\n");
				delete [] vertex;
				delete [] triangle;
				delete [] weight;
				return 1;
			}

			weight[i].joint = joint;
			weight[i].bias = bias;
			weight[i].pos.x = x;
			weight[i].pos.y = y;
			weight[i].pos.z = z;
		}
		else
		{
			debugf("Error reading weight %d\n", i);
			delete [] vertex;
			delete [] triangle;
			delete [] weight;
			return 1;
		}
		pdata = strstr(pdata + 7, "weight ");
	}


	// initialize struct
	mesh->vertex = vertex;
	mesh->triangle = triangle;
	mesh->weight = weight;
	mesh->num_vertex = num_vertex;
	mesh->num_tri = num_tri;
	mesh->num_weight = num_weight;
	strcpy(mesh->shader, shader + 1);
	mesh->shader[strlen(mesh->shader) - 1] = '\0';
	return 0;
}

int MD5::load_md5_animation(char *file, anim_list_t *plist)
{
	char *data = get_file(file, NULL);
	char *pdata = NULL;

	int version;
	int num_frame;
	int num_joint;
	int frame_rate;
	int num_ani;

	md5_hierarchy_t *hierarchy;
	md5_aabb_t	*aabb;
	md5_base_t	*base;
	float		*frame;

	if (data == NULL)
	{
		printf("Unable to open %s\n", file);
		return -1;
	}

	pdata = strstr(data, "MD5Version");
	sscanf(pdata, "MD5Version %d", &version);

	pdata = strstr(data, "numFrames");
	sscanf(pdata, "numFrames %d", &num_frame);

	pdata = strstr(data, "numJoints");
	sscanf(pdata, "numJoints %d", &num_joint);

	// Usually 24 frames, need to interpolate for higher rates
	pdata = strstr(data, "frameRate");
	sscanf(pdata, "frameRate %d", &frame_rate);

	pdata = strstr(data, "numAnimatedComponents");
	sscanf(pdata, "numAnimatedComponents %d", &num_ani);


	hierarchy = new md5_hierarchy_t[num_joint];
	if (hierarchy == NULL)
	{
		perror("malloc() failed");
		return 1;
	}


	aabb = new md5_aabb_t[num_frame];
	if (aabb == NULL)
	{
		perror("malloc() failed");
		delete [] hierarchy;
		return 1;
	}

	base = new md5_base_t[num_joint];
	if (base == NULL)
	{
		perror("malloc() failed");
		delete [] hierarchy;
		delete [] aabb;
		return 1;
	}

	frame = new float[num_frame * num_ani];
	if (frame == NULL)
	{
		perror("malloc() failed");
		delete [] hierarchy;
		delete [] aabb;
		delete [] base;
		return 1;
	}


	pdata = strstr(data, "hierarchy {");
	if (parse_hierarchy(pdata, num_joint, hierarchy))
	{
		delete [] hierarchy;
		delete [] aabb;
		delete [] base;
		delete [] frame;
		return 1;
	}

	//	FILE *out = fopen("hierarchy.bin", "wb");
	//	fwrite(hierarchy, sizeof(md5_hierarchy_t), num_joint, out);
	//	fclose(out);



	pdata = strstr(data, "bounds {");
	if (parse_bounds(pdata, num_frame, aabb))
	{
		delete [] hierarchy;
		delete [] aabb;
		delete [] base;
		delete [] frame;
		return 1;
	}

	//	out = fopen("aabb.bin", "wb");
	//	fwrite(aabb, sizeof(md5_aabb_t), num_frame, out);
	//	fclose(out);

	pdata = strstr(data, "baseframe {");
	if (parse_base(pdata, num_joint, base))
	{
		delete [] hierarchy;
		delete [] aabb;
		delete [] base;
		delete [] frame;
		return 1;
	}

	//	out = fopen("base.bin", "wb");
	//	fwrite(base, sizeof(md5_base_t), num_joint, out);
	//	fclose(out);

	pdata = strstr(data, "frame 0 {");
	if (parse_frame(pdata, num_frame, num_ani, frame))
	{
		delete [] hierarchy;
		delete [] aabb;
		delete [] base;
		delete [] frame;
		return 1;
	}

	while (1)
	{
		if (plist->anim == NULL)
		{
			break;
		}
		if (plist->next)
		{
			plist = plist->next;
		}
		else
		{
			plist->next = new anim_list_t;
			plist_stack.push(plist->next);
			memset(plist->next, 0, sizeof(anim_list_t));
			strcpy(plist->next->name, "empty");
			plist = plist->next;
			break;
		}
	}

	plist->anim = new md5_anim_t;

	memcpy(plist->name, file, strlen(file) + 1);
	plist->anim->num_frame = num_frame;
	plist->anim->num_joint = num_joint;
	plist->anim->frame_rate = frame_rate;

	plist->anim->aabb = aabb;
	plist->anim->base = base;
	plist->anim->num_ani = num_ani;
	plist->anim->hierarchy = hierarchy;
	plist->anim->frame = frame;
	delete [] data;
	return 0;
}

int MD5::parse_hierarchy(char *data, int num_joint, md5_hierarchy_t *hierarchy)
{
	char name[64];
	char *pdata;

	int i;

	pdata = strchr(data, '\"');

	for(i = 0; i < num_joint; i++)
	{
		int parent;
		int flag;
		int start;

		if (pdata == NULL)
		{
			printf("Error: Unexpected end of hierarchy\n");
			return 1;
		}
	

		if ( 4 == sscanf(pdata, "%s %d %d %d", name, &parent, &flag, &start) )
		{
			strcpy(hierarchy[i].name, name);
			hierarchy[i].parent = parent;
			hierarchy[i].flag = flag;
			hierarchy[i].start = start;
		}
		else
		{
			printf("Error: Unexpected end of hierarchy\n");
			return 1;
		}
		pdata = strchr(pdata + 1, '\"');
		pdata = strchr(pdata + 1, '\"');
	}
	return 0;
}

int MD5::parse_bounds(char *data, int num_bound, md5_aabb_t *aabb)
{
	char *pdata;
	int i = 0;


	pdata = strchr(data, '(');
	for(i = 0; i < num_bound; i++)
	{
		float x1, y1, z1;
		float x2, y2, z2;

		if (pdata == NULL)
		{
			printf("Error: Unexpected end of bound\n");
			return 1;
		}

		if ( 6 == sscanf(pdata, "( %f %f %f ) ( %f %f %f )", &x1, &y1, &z1, &x2, &y2, &z2) )
		{
			aabb[i].min.x = x1;
			aabb[i].min.y = y1;
			aabb[i].min.z = z1;
			aabb[i].max.x = x2;
			aabb[i].max.y = y2;
			aabb[i].max.z = z2;
		}
		else
		{
			printf("Unable to read bound %d\n", i);
			return 1;
		}
		pdata = strchr(pdata + 1, '(');
		pdata = strchr(pdata + 1, '(');
	}
	return 0;
}

int MD5::parse_base(char *data, int num_base, md5_base_t *base)
{
	char *pdata;
	float x, y, z;
	float qx, qy, qz; 
	int i;


	pdata = strchr(data, '(');
	for(i = 0; i < num_base; i++)
	{
		if ( 6 == sscanf(pdata, "( %f %f %f ) ( %f %f %f )", &x, &y, &z, &qx, &qy, &qz) )
		{
			base[i].pos.x = x;
			base[i].pos.y = y;
			base[i].pos.z = z;
			base[i].orient.x = qx;
			base[i].orient.y = qy;
			base[i].orient.z = qz;
			base[i].orient.compute_w();
		}
		else
		{
			printf("Unable to read base %d\n", i);
			return 1;
		}
		pdata = strchr(pdata + 1, '(');
		pdata = strchr(pdata + 1, '(');
	}
	return 0;
}

int MD5::parse_frame(char *data, int num_frame, int num_ani, float *frame)
{
	char *pdata = data;
	float	f;
	int	i;

	for(int j = 0; j < num_frame; j++)
	{
		pdata = strchr(pdata, '{');
		pdata++;

		//skip whitespace
		while (*pdata == ' ' || *pdata == '\n' || *pdata == '\r' || *pdata == '\t')
			pdata++;
			
		for(i = 0; i < num_ani; i++)
		{
			f = (float)atof(pdata);
			frame[num_ani * j + i] = f;

			//find whitespace
			while (*pdata != ' ' && *pdata != '\n' && *pdata != '\r' && *pdata != '\t')
				pdata++;

			//skip whitespace
			while (*pdata == ' ' || *pdata == '\n' || *pdata == '\r' || *pdata == '\t')
				pdata++;
		}

//		char fname[80];
//		sprintf(fname, "frame%00d.bin", j);
//		FILE *out = fopen(fname, "wb");
//		fwrite(&frame[num_ani * j], sizeof(float), num_ani, out);
//		fclose(out);
	
		pdata = strstr(pdata, "frame ");
	}
	return 0;
}


void MD5::build_frame(md5_joint_t *joint, float *frame, md5_anim_t *anim)
{
	int i;

	for (i = 0; i < anim->num_joint; i++)
	{
		strcpy(joint[i].name, anim->hierarchy[i].name);
		joint[i].parent = anim->hierarchy[i].parent;
		joint[i].pos = anim->base[i].pos;
		joint[i].orient = anim->base[i].orient;
		int j = 0;


		if (anim->hierarchy[i].flag & 1)
		{
			//replace x
			joint[i].pos.x = frame[anim->hierarchy[i].start + j++];
		}
		if (anim->hierarchy[i].flag & 2)
		{
			//replace y
			joint[i].pos.y = frame[anim->hierarchy[i].start + j++];
		}
		if (anim->hierarchy[i].flag & 4)
		{
			//replace z
			joint[i].pos.z = frame[anim->hierarchy[i].start + j++];
		}
		if (anim->hierarchy[i].flag & 8)
		{
			//qx
			joint[i].orient.x = frame[anim->hierarchy[i].start + j++];
		}
		if (anim->hierarchy[i].flag & 16)
		{
			//qy
			joint[i].orient.y = frame[anim->hierarchy[i].start + j++];
		}
		if (anim->hierarchy[i].flag & 32)
		{
			//qz
			joint[i].orient.z = frame[anim->hierarchy[i].start + j++];
		}
		joint[i].orient.compute_w();


		// parent will always be less than index i
		// parent < i
		if (joint[i].parent >= 0)
		{
			// Need to rotate our position by parents orientation
			quaternion q(0, joint[i].pos);
			q = (joint[joint[i].parent].orient * q) * joint[joint[i].parent].orient.conjugate();

			joint[i].pos.x = q.x + joint[joint[i].parent].pos.x;
			joint[i].pos.y = q.y + joint[joint[i].parent].pos.y;
			joint[i].pos.z = q.z + joint[joint[i].parent].pos.z;
			joint[i].orient = joint[joint[i].parent].orient * joint[i].orient;
			joint[i].orient.normalize();
		}
	}
}

void MD5::calc_tangent(vertex_t &a, vertex_t &b, vertex_t &c)
{
	vec3 p = b.position - a.position;
	vec3 q = c.position - a.position;
	vec3 n = vec3::crossproduct(p, q);
	vec3 t;
	vec3 bi;
	float s1, s2;
	float t1, t2;
	float denom;

	a.normal += n;
	b.normal += n;
	c.normal += n;

	s1 = b.texCoord0.x - a.texCoord0.x;
	t1 = b.texCoord0.y - a.texCoord0.y;

	s2 = c.texCoord0.x - a.texCoord0.x;
	t2 = c.texCoord0.y - a.texCoord0.y;

	denom = (s1 * t2 - s2 * t1);
	if (denom != 0)
	{
		t = (p * t2 - q * t1) / denom;
		bi = (q * s1 - p * s2) / denom;
	}
	else
	{
		t = vec3(1.0f, 0.0f, 0.0f);
		bi = vec3(0.0f, 0.0f, 1.0f);
	}

	a.tangent += vec4(t.x, t.y, t.z, 0);
	b.tangent += vec4(t.x, t.y, t.z, 0);
	c.tangent += vec4(t.x, t.y, t.z, 0);
}

void MD5::generate_tangent(int *index_array, int num_index, vertex_t *vertex_array, int num_vertex)
{
	for(int i = 0; i < num_index; i++)
	{
		vertex_array[i].normal = vec3(0.0f, 0.0f, 0.0f);
		vertex_array[i].tangent = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	for(int i = 0; i < num_index; i += 3)
	{
		calc_tangent(vertex_array[i], vertex_array[i+1], vertex_array[i+2]);
	}

	for(int i = 0; i < num_index; i++)
	{
		vertex_array[i].normal *= (1.0f / 3.0f);
		vertex_array[i].tangent *= (1.0f / 3.0f);
		vertex_array[i].tangent.w = 1.0f;
	}


}


void MD5::generate_animation(md5_joint_t **&frame, md5_anim_t *anim)
{
	frame = new md5_joint_t *[anim->num_frame];
	for(int i = 0; i < anim->num_frame; i++)
	{
		frame[i] = new md5_joint_t [anim->num_joint];
		build_frame(frame[i], &anim->frame[i * anim->num_ani], anim);
	}
}

void MD5::destroy_animation(md5_joint_t **&frame, md5_anim_t *anim)
{
	for(int i = 0; i < anim->num_frame; i++)
	{
		delete [] frame[i];
	}
	delete [] frame;
}
