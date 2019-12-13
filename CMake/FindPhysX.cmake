include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for PhysX SDK
#-----------------------------------------------------------------------------

set(physx_release_type)
if(NOT PHYSX_CONFIGURATION STREQUAL "RELEASE")
  set(physx_release_type ${PHYSX_CONFIGURATION})
endif()

imstk_find_header(PhysX PxPhysicsAPI.h physx)
imstk_find_libary(PhysX PhysX _static${physx_release_type}_64 _staticDEBUG_64)#Different release and debug postfixes
imstk_find_libary(PhysX PhysXCharacterKinematic _static${physx_release_type}_64 _staticDEBUG_64)#Different release and debug postfixes
imstk_find_libary(PhysX PhysXCommon _static${physx_release_type}_64 _staticDEBUG_64)#Different release and debug postfixes
imstk_find_libary(PhysX PhysXCooking _static${physx_release_type}_64 _staticDEBUG_64)#Different release and debug postfixes
imstk_find_libary(PhysX PhysXExtensions _static${physx_release_type}_64 _staticDEBUG_64)#Different release and debug postfixes
imstk_find_libary(PhysX PhysXFoundation _static${physx_release_type}_64 _staticDEBUG_64)#Different release and debug postfixes
imstk_find_libary(PhysX PhysXPvdSDK _static${physx_release_type}_64 _staticDEBUG_64)#Different release and debug postfixes
imstk_find_libary(PhysX PhysXVehicle _static${physx_release_type}_64 _staticDEBUG_64)#Different release and debug postfixes

imstk_find_package(PhysX)

message(STATUS "PhysX include : ${PHYSX_INCLUDE_DIRS}")
message(STATUS "PhysX libraries : ${PHYSX_LIBRARIES}")
