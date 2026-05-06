template<typename T>
ego::Reference<T>::Reference(nullptr_t)
	: m_object(nullptr)
{}

template<typename T>
ego::Reference<T>::Reference(T* _object)
{
	if (_object)
	{
		const_cast<T*>(_object)->addReference();
		m_object = _object;
	}
}

template<typename T>
ego::Reference<T>::Reference(const Reference<T>& _reference)
{
	if (_reference.m_object)
	{
		const_cast<T*>(_reference.m_object)->addReference();
		m_object = _reference.m_object;
	}
}

template<typename T>
template<typename U, typename>
ego::Reference<T>::Reference(const ego::Reference<U>& _reference)
	: Reference<T>(std::move(Reference(_reference.getObject()))) {}

template<typename T>
ego::Reference<T>::~Reference()
{
	reset();
}

template <typename T>
ego::Reference<T>& ego::Reference<T>::operator=(const Reference<T>& _reference)
{
	assign(const_cast<T*>(_reference.m_object));
	return *this;
}

template <typename T1, typename T2>
bool operator<(ego::Reference<T1> _reference1, ego::Reference<T2> _reference2)
{
	return _reference1->getObject() < _reference2->getObject();
}

template <typename T1, typename T2>
bool operator>(ego::Reference<T1> _reference1, ego::Reference<T2> _reference2)
{
	return _reference1->getObject() > _reference2->getObject();
}

template <typename T>
void ego::Reference<T>::reset()
{
	release();
}

template<typename T>
void ego::Reference<T>::release()
{
	if (m_object)
	{
		const_cast<T*>(m_object)->releaseReference();
		m_object = nullptr;
	}
}

template <typename T>
void ego::Reference<T>::assign(T* _object)
{
	if (m_object == _object)
	{
		return;
	}

	release();

	if (_object)
	{
		const_cast<T*>(_object)->addReference();
		m_object = _object;
	}
}