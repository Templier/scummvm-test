// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsd�rfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#ifndef BS_REGIONREGISTRY_H
#define BS_REGIONREGISTRY_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "kernel/common.h"
#include "kernel/persistable.h"
#include "kernel/objectregistry.h"

#include "kernel/memlog_off.h"
#include <memory>
#include "kernel/memlog_on.h"

// -----------------------------------------------------------------------------
// Forward Deklarationen
// -----------------------------------------------------------------------------

class BS_Region;

// -----------------------------------------------------------------------------
// Klassendeklaration
// -----------------------------------------------------------------------------

class BS_RegionRegistry : public BS_ObjectRegistry<BS_Region>, public BS_Persistable
{
public:
	static BS_RegionRegistry & GetInstance()
	{
		if (!m_InstancePtr.get()) m_InstancePtr.reset(new BS_RegionRegistry);
		return *m_InstancePtr.get();
	}

	virtual bool Persist(BS_OutputPersistenceBlock & Writer);
	virtual bool Unpersist(BS_InputPersistenceBlock & Reader);

private:
	virtual void LogErrorLn(const char * Message) const;
	virtual void LogWarningLn(const char * Message) const;

	static std::auto_ptr<BS_RegionRegistry> m_InstancePtr;
};

#endif
