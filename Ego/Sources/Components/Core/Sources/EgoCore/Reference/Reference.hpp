template<typename T>
ego::Reference<T>::Reference(std::nullptr_t)
	: m_object(nullptr)
{}

template<typename T>
void ego::Reference<T>::validateReferenceType()
{
	static_assert(
		std::is_base_of_v<MTCountable, T>,
		"ego::Reference<T> requires T to derive from ego::MTCountable"
	);
}

template<typename T>
ego::Reference<T>::Reference(T* _object)
{
	validateReferenceType();
	assign(_object);
}

template<typename T>
ego::Reference<T>::Reference(const Reference<T>& _reference)
{
	validateReferenceType();
	assign(_reference.m_object);
}

template<typename T>
ego::Reference<T>::Reference(Reference<T>&& _reference) noexcept
	: m_object(std::exchange(_reference.m_object, nullptr))
{
	validateReferenceType();
}

template<typename T>
template<typename U, typename>
ego::Reference<T>::Reference(const ego::Reference<U>& _reference)
	: Reference<T>(static_cast<T*>(_reference.m_object))
{}

template<typename T>
template<typename U, typename>
ego::Reference<T>::Reference(ego::Reference<U>&& _reference) noexcept
	: m_object(static_cast<T*>(std::exchange(_reference.m_object, nullptr)))
{
	validateReferenceType();
}

template<typename T>
ego::Reference<T>::~Reference()
{
	reset();
}

template <typename T>
ego::Reference<T>& ego::Reference<T>::operator=(std::nullptr_t)
{
	reset();
	return *this;
}

template <typename T>
ego::Reference<T>& ego::Reference<T>::operator=(T* _object)
{
	assign(_object);
	return *this;
}

template <typename T>
ego::Reference<T>& ego::Reference<T>::operator=(const Reference<T>& _reference)
{
	if (this != &_reference)
	{
		assign(_reference.m_object);
	}

	return *this;
}

template <typename T>
ego::Reference<T>& ego::Reference<T>::operator=(Reference<T>&& _reference) noexcept
{
	if (this != &_reference)
	{
		release();
		m_object = std::exchange(_reference.m_object, nullptr);
	}

	return *this;
}

template<typename T>
template<typename U, typename>
ego::Reference<T>& ego::Reference<T>::operator=(const ego::Reference<U>& _reference)
{
	assign(static_cast<T*>(_reference.m_object));
	return *this;
}

template<typename T>
template<typename U, typename>
ego::Reference<T>& ego::Reference<T>::operator=(ego::Reference<U>&& _reference) noexcept
{
	release();
	m_object = static_cast<T*>(std::exchange(_reference.m_object, nullptr));
	return *this;
}

template <typename T>
void ego::Reference<T>::reset()
{
	release();
}

template<typename T>
void ego::Reference<T>::release()
{
	validateReferenceType();

	if (m_object)
	{
		T* object = m_object;
		m_object = nullptr;
		object->releaseReference();
	}
}

template <typename T>
void ego::Reference<T>::assign(T* _object)
{
	validateReferenceType();

	if (m_object == _object)
	{
		return;
	}

	if (_object)
	{
		_object->addReference();
	}

	release();
	m_object = _object;
}

template <typename T>
void ego::Reference<T>::swap(Reference& _reference) noexcept
{
	std::swap(m_object, _reference.m_object);
}

template <typename T1, typename T2>
bool ego::operator==(const Reference<T1>& _reference1, const Reference<T2>& _reference2)
{
	return static_cast<const void*>(_reference1.getObject())
		== static_cast<const void*>(_reference2.getObject());
}

template <typename T1, typename T2>
bool ego::operator!=(const Reference<T1>& _reference1, const Reference<T2>& _reference2)
{
	return !(_reference1 == _reference2);
}

template <typename T1, typename T2>
bool ego::operator<(const Reference<T1>& _reference1, const Reference<T2>& _reference2)
{
	return std::less<const void*>()(
		static_cast<const void*>(_reference1.getObject()),
		static_cast<const void*>(_reference2.getObject())
	);
}

template <typename T1, typename T2>
bool ego::operator>(const Reference<T1>& _reference1, const Reference<T2>& _reference2)
{
	return _reference2 < _reference1;
}

template <typename T1, typename T2>
bool ego::operator<=(const Reference<T1>& _reference1, const Reference<T2>& _reference2)
{
	return !(_reference2 < _reference1);
}

template <typename T1, typename T2>
bool ego::operator>=(const Reference<T1>& _reference1, const Reference<T2>& _reference2)
{
	return !(_reference1 < _reference2);
}
