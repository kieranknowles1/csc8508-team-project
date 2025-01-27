
namespace NCL::Rendering {
	struct OGLBuffer {
		OGLBuffer() {
			glGenBuffers(1, &gpuID);
		}

		virtual ~OGLBuffer() {
			glDeleteBuffers(1, &gpuID);
		}
		GLuint gpuID;
	};

	template <typename T>
	struct StructuredOGLBuffer : OGLBuffer{
		StructuredOGLBuffer(size_t count = 1) {
			elements = new T[count];
			GLuint flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT;

			dataSize = sizeof(T) * count;

			glBindBuffer(GL_ARRAY_BUFFER, gpuID);
			glBufferStorage(GL_ARRAY_BUFFER, dataSize, 0, flags);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			GPUSync(); //T might have a sensible constructor
		}
		~StructuredOGLBuffer() {
			delete elements;
		}

		void GPUSync() {
			glBindBuffer(GL_ARRAY_BUFFER, gpuID);
			glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, elements);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		T* elements;
		size_t dataSize;
	};
}