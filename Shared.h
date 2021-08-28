#ifndef SHARED_H
#define SHARED_H

///
/// \brief Deletes \p _x and sets it to nullptr.
///
/// \note  It seems this is considered silly, but I think it's helpful if _a is a reference to a member.
///
/// \param _x - Reference to pointer to object that is to be deleted
///
template<typename T>
void SafeDelete(T*& _x)
{
    // We can skip the nullptr check because apparently c++ does it for you in like this one instance.
    delete _x;
    _x = nullptr;
}

#endif // SHARED_H
