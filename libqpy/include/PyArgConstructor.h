#pragma once
// QPy - Copyright (c) 2012,2013 Ugo Varetto
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the author and copyright holder nor the
//       names of contributors to the project may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL UGO VARETTO BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/// @brief Declarations and definitions of constructors for creating PyObjects
/// from C++ values.

#include <Python.h>

#include <QGenericReturnArgument>
#include <QMetaType>

namespace qpy {
/// @brief Abstract base class for return constructors which create PyObjects
/// from C++ values.
///
/// There are two cases where data need to converted from Qt-C++ to Python:
/// -# Python function/method invocation as a result of Qt signal triggered
/// -# Return value of Qt-C++ method invoked from Python
///
/// In the case of (1) each parameter is directly converted to its Python
/// format; in the case of (2) the return value must first be stored into
/// a memory location passed to the method invocation function, and then
/// be converted into Python format.
class PyArgConstructor {
public:
    /// @brief Create PyObject from value returned from QObject method.
    virtual PyObject* Create() const = 0;
    /// @brief Create PyObject from parameter passed to Python callback
    /// from Qt when signal triggered
    virtual PyObject* Create( void* ) const = 0;
    /// Virtual destructor.
    virtual ~PyArgConstructor() {}
    /// Return copy of object.
    virtual PyArgConstructor* Clone() const = 0;
    /// Return type of constructed data.
    virtual QMetaType::Type Type() const = 0;
    /// @brief Return QGenericReturnArguments holding a reference to the
    /// memory location where the returned value is stored.
    QGenericReturnArgument Argument() const { return ga_; }
    /// @brief Return @c true if type is a pointer to a QObject-derived object.
    ///
    /// This is required to have the QPy run-time add the passed QObject into
    /// the Python context. The other option is to have PyArgConstructors::Create
    /// receive a reference to a PyContext which introduces a two-way
    /// dependency between PyArgConstructor and PyContext; this would
    /// also augment PyQArgConstructor requirements with the added responsibility
    /// of having to add instances to Python.
    /// Note that it is not enough to simply check the PyArgConstructor returned
    /// Type(), because in the case of custom registered types derived from QObject
    /// the returned type is not QObjectStar. 
    virtual bool IsQObjectPtr() const = 0;
protected:
    /// Creates return argument of the proper type.
    template < typename T > void SetArg( T& arg ) {
        ga_ = QReturnArgument< T >( QMetaType::typeName( Type() ), arg );
    }
private:    
    /// Placeholder for returned data. 
    QGenericReturnArgument ga_; // not private, breaks encapsulation
};

/// @brief Use this type to specify that no Qt -> Python conversion exists for
/// a type when registering types through PyContext
///
/// e.g. 
/// @code
/// RegisterType< VoidStarQArgConstructor,
///               NoPyArgConstructor >( QMetaType::VoidStar );
/// @endcode    
class NoPyArgConstructor : public PyArgConstructor {
public:
    PyObject* Create( void*  ) const {
        Py_RETURN_NONE;
    }
    PyObject* Create() const {
        Py_RETURN_NONE;
    }
    NoPyArgConstructor* Clone() const {
        return new NoPyArgConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Void; }
    bool IsQObjectPtr() const { return false; }
};
typedef NoPyArgConstructor NO_PY_ARG;
}