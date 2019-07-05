# C++ Schema Codegen (with C serialization interop)

Build with .NET Core 2.2.102

> Only tested on Windows.

* Built on top of Jared's codegen framework (current exists [here](https://github.com/improbable/dotnet_core_worker/tree/feature/new-inventory-worker)).
* Visual Studio solution which can be built out to an executable or used in-editor to debug.
* Parameterized by an input schema bundle and desired output directory for generated code.
* No guaranteed resilience for different SpatialOS versions, current implementation is `13.6.0` (would suggest changing the version in `./Test.sh`, running it, and observing any compile errors).
* This is intended as both a building block for customers to build on top of, and reference for those wishing to build code generators from scratch.
* This lacks even the most basic testing.
* Few things to fix still: mutual type referencing leading to circular includes, catching for using C++ keywords as fields, fields and events whose generated names collide, components in utils namespace that collide with helper functions.

## Building the solution

The executable can be created by either:
* Running the `Setup.bat` script.
* Building the `Improbable.Unreal.Scripts.sln` solution in JetBrains Rider, Visual Studio, Visual Studio Code.

## Running from Visual Studio

For debugging purposes, the code generator can be run from Visual Studio. To do this, you'll need to add values for the `input-bundle` and `output-dir` parameters.

This can be done through right-clicking the `CodeGenerator` project in the Solution Explorer, and inserting into `Properties -> Debug -> Application arguments`.

## Example Output

For each type defined in a SpatialOS schema file, a header and source are generated. For example. below are schema files generated from the TestComponent component schema:

```
component TestComponent {
  id = 198703;
  data TestComponentData;

  [EmptyType]
  event TestComponentData.EmptyEvent empty;
  event TestComponentData.ParameterizedEvent params;

  [EmptyType]
  command Integer test_command(Integer);
  command Nested.Recursive another_test_command(Integer);
}
```

### TestComponent.h

```cpp
namespace improbable {
namespace test_schema {
// Generated from Testing/Schema/test.schema(57,1)
class TestComponent
{
public:
	static const Worker_ComponentId ComponentId = 198703;

	// Creates a new instance with specified arguments for each field.
	TestComponent(std::int32_t TestInt, float TestFloat, bool TestBool, double TestDouble, ::improbable::List<double> TestList, ::improbable::Map<std::string, double> TestMap, std::vector<std::uint8_t> TestBytes);
	// Creates a new instance with default values for each field.
	TestComponent();
	// Creates a new instance with default values for each field. This is
	// equivalent to a default-constructed instance.
	static TestComponent Create() { return {}; }
	// Copyable and movable.
	TestComponent(TestComponent&&) = default;
	TestComponent(const TestComponent&) = default;
	TestComponent& operator=(TestComponent&&) = default;
	TestComponent& operator=(const TestComponent&) = default;
	~TestComponent() = default;

	bool operator==(const TestComponent&) const;
	bool operator!=(const TestComponent&) const;

	// Serialize this object data into the C API argument
	void serialize(Schema_ComponentData* component_data) const;

	// Deserialize the C API object argument into an instance of this class and return it
	static TestComponent deserialize(Schema_ComponentData* component_data);

	// Field test_int = 1
	std::int32_t get_test_int() const;
	std::int32_t& get_test_int();
	TestComponent& set_test_int(std::int32_t);

	// Field test_float = 2
	float get_test_float() const;
	float& get_test_float();
	TestComponent& set_test_float(float);

	// Field test_bool = 3
	bool get_test_bool() const;
	bool& get_test_bool();
	TestComponent& set_test_bool(bool);

	// Field test_double = 4
	double get_test_double() const;
	double& get_test_double();
	TestComponent& set_test_double(double);

	// Field test_list = 5
	const ::improbable::List<double>& get_test_list() const;
	::improbable::List<double>& get_test_list();
	TestComponent& set_test_list(const ::improbable::List<double>&);

	// Field test_map = 6
	const ::improbable::Map<std::string, double>& get_test_map() const;
	::improbable::Map<std::string, double>& get_test_map();
	TestComponent& set_test_map(const ::improbable::Map<std::string, double>&);

	// Field test_bytes = 7
	std::vector<std::uint8_t> get_test_bytes() const;
	std::vector<std::uint8_t>& get_test_bytes();
	TestComponent& set_test_bytes(std::vector<std::uint8_t>);

private:
	std::int32_t _test_int;
	float _test_float;
	bool _test_bool;
	double _test_double;
	::improbable::List<double> _test_list;
	::improbable::Map<std::string, double> _test_map;
	std::vector<std::uint8_t> _test_bytes;

public:
	class Update
	{
		// Creates a new instance with default values for each field.
		Update();
		// Creates a new instance with default values for each field. This is
		// equivalent to a default-constructed instance.
		static Update Create() { return {}; }
		// Copyable and movable.
		Update(Update&&) = default;
		Update(const Update&) = default;
		Update& operator=(Update&&) = default;
		Update& operator=(const Update&) = default;
		~Update() = default;
		bool operator==(const Update&) const;
		bool operator!=(const Update&) const;

		// Creates an Update from a ::improbable::test_schema::TestComponent object.
		static Update FromInitialData(const ::improbable::test_schema::TestComponent& data);

		/**
		  * Converts to a ::improbable::test_schema::TestComponent
		  * object. It is an error to call this function unless *all* of the optional fields in this
		  * update are filled in.
		  */
		::improbable::test_schema::TestComponent ToInitialData() const;

		/**
		  * Replaces fields in the given ::improbable::test_schema::TestComponent
		  * object with the corresponding fields in this update, where present.
		  */
		void ApplyTo(::improbable::test_schema::TestComponent&) const;

		// Serialize this update object data into the C API component update argument
		void serialize(Schema_ComponentUpdate* component_update) const;

		// Deserialize the C API component update argument into an instance of this class and return it
		static Update deserialize(Schema_ComponentUpdate* component_update);

		// Field test_int = 1
		const ::improbable::Option<std::int32_t>& get_test_int() const;
		::improbable::Option<std::int32_t>& get_test_int();
		TestComponent::Update& set_test_int(std::int32_t);

		// Field test_float = 2
		const ::improbable::Option<float>& get_test_float() const;
		::improbable::Option<float>& get_test_float();
		TestComponent::Update& set_test_float(float);

		// Field test_bool = 3
		const ::improbable::Option<bool>& get_test_bool() const;
		::improbable::Option<bool>& get_test_bool();
		TestComponent::Update& set_test_bool(bool);

		// Field test_double = 4
		const ::improbable::Option<double>& get_test_double() const;
		::improbable::Option<double>& get_test_double();
		TestComponent::Update& set_test_double(double);

		// Field test_list = 5
		const ::improbable::Option<::improbable::List<double>>& get_test_list() const;
		::improbable::Option<::improbable::List<double>>& get_test_list();
		TestComponent::Update& set_test_list(::improbable::List<double>);

		// Field test_map = 6
		const ::improbable::Option<::improbable::Map<std::string, double>>& get_test_map() const;
		::improbable::Option<::improbable::Map<std::string, double>>& get_test_map();
		TestComponent::Update& set_test_map(::improbable::Map<std::string, double>);

		// Field test_bytes = 7
		const ::improbable::Option<std::vector<std::uint8_t>>& get_test_bytes() const;
		::improbable::Option<std::vector<std::uint8_t>>& get_test_bytes();
		TestComponent::Update& set_test_bytes(std::vector<std::uint8_t>);

		// Event empty = 1
		const ::improbable::List<::improbable::test_schema::TestComponentData::EmptyEvent>& get_empty_list() const;
		::improbable::List<::improbable::test_schema::TestComponentData::EmptyEvent>& get_empty_list();
		TestComponent::Update& add_empty(const ::improbable::test_schema::TestComponentData::EmptyEvent&);

		// Event params = 2
		const ::improbable::List<::improbable::test_schema::TestComponentData::ParameterizedEvent>& get_params_list() const;
		::improbable::List<::improbable::test_schema::TestComponentData::ParameterizedEvent>& get_params_list();
		TestComponent::Update& add_params(const ::improbable::test_schema::TestComponentData::ParameterizedEvent&);


	private:
		::improbable::Option<std::int32_t> _test_int;
		::improbable::Option<float> _test_float;
		::improbable::Option<bool> _test_bool;
		::improbable::Option<double> _test_double;
		::improbable::Option<::improbable::List<double>> _test_list;
		::improbable::Option<::improbable::Map<std::string, double>> _test_map;
		::improbable::Option<std::vector<std::uint8_t>> _test_bytes;
		::improbable::List<::improbable::test_schema::TestComponentData::EmptyEvent> _empty_list;
		::improbable::List<::improbable::test_schema::TestComponentData::ParameterizedEvent> _params_list;
	};


	class Commands
	{
		class TestCommand
		{
		public:
			static const uint32_t CommandId = 1;
			using Request = ::improbable::test_schema::Integer;
			using Response = ::improbable::test_schema::Integer;
		};
		class AnotherTestCommand
		{
		public:
			static const uint32_t CommandId = 2;
			using Request = ::improbable::test_schema::Integer;
			using Response = ::improbable::test_schema::Nested::Recursive;
		};
	};
};

} // namespace test_schema
} // namespace improbable
```

### TestComponent.cpp

```cpp
namespace improbable {
namespace test_schema {

TestComponent::TestComponent(
	std::int32_t test_int,
	float test_float,
	bool test_bool,
	double test_double,
	::improbable::List<double> test_list,
	::improbable::Map<std::string, double> test_map,
	std::vector<std::uint8_t> test_bytes)
: _test_int{ test_int }
, _test_float{ test_float }
, _test_bool{ test_bool }
, _test_double{ test_double }
, _test_list{ test_list }
, _test_map{ test_map }
, _test_bytes{ test_bytes } {}

TestComponent::TestComponent() {}

bool TestComponent::operator==(const TestComponent& value) const
{
	return _test_int == value._test_int &&
	_test_float == value._test_float &&
	_test_bool == value._test_bool &&
	_test_double == value._test_double &&
	_test_list == value._test_list &&
	_test_map == value._test_map &&
	_test_bytes == value._test_bytes;
}

bool TestComponent::operator!=(const TestComponent& value) const
{
	return !operator== (value);
}

std::int32_t TestComponent::get_test_int() const
{
	return _test_int;
}

std::int32_t& TestComponent::get_test_int()
{
	return _test_int;
}

TestComponent& TestComponent::set_test_int(std::int32_t value)
{
	_test_int = value;
	return *this;
}
float TestComponent::get_test_float() const
{
	return _test_float;
}

float& TestComponent::get_test_float()
{
	return _test_float;
}

TestComponent& TestComponent::set_test_float(float value)
{
	_test_float = value;
	return *this;
}
bool TestComponent::get_test_bool() const
{
	return _test_bool;
}

bool& TestComponent::get_test_bool()
{
	return _test_bool;
}

TestComponent& TestComponent::set_test_bool(bool value)
{
	_test_bool = value;
	return *this;
}
double TestComponent::get_test_double() const
{
	return _test_double;
}

double& TestComponent::get_test_double()
{
	return _test_double;
}

TestComponent& TestComponent::set_test_double(double value)
{
	_test_double = value;
	return *this;
}
const ::improbable::List<double>& TestComponent::get_test_list() const
{
	return _test_list;
}

::improbable::List<double>& TestComponent::get_test_list()
{
	return _test_list;
}

TestComponent& TestComponent::set_test_list(const ::improbable::List<double>& value)
{
	_test_list = value;
	return *this;
}
const ::improbable::Map<std::string, double>& TestComponent::get_test_map() const
{
	return _test_map;
}

::improbable::Map<std::string, double>& TestComponent::get_test_map()
{
	return _test_map;
}

TestComponent& TestComponent::set_test_map(const ::improbable::Map<std::string, double>& value)
{
	_test_map = value;
	return *this;
}
std::vector<std::uint8_t> TestComponent::get_test_bytes() const
{
	return _test_bytes;
}

std::vector<std::uint8_t>& TestComponent::get_test_bytes()
{
	return _test_bytes;
}

TestComponent& TestComponent::set_test_bytes(std::vector<std::uint8_t> value)
{
	_test_bytes = value;
	return *this;
}

void TestComponent::serialize(Schema_ComponentData* component_data) const
{
	Schema_Object* fields_objects = Schema_GetComponentDataFields(component_data);
	// serializing field test_int = 1
	Schema_AddInt32(fields_objects, 1, _test_int);

	// serializing field test_float = 2
	Schema_AddFloat(fields_objects, 2, _test_float);

	// serializing field test_bool = 3
	Schema_AddBool(fields_objects, 3, static_cast<std::uint8_t>(_test_bool));

	// serializing field test_double = 4
	Schema_AddDouble(fields_objects, 4, _test_double);

	// serializing field test_list = 5
	for (std::uint32_t i = 0; i < _test_list.size(); ++i)
	{
		Schema_AddDouble(fields_objects, 5, _test_list[i]);;
	}

	// serializing field test_map = 6
	for (auto _it=_test_map.begin(); _it!=_test_map.end(); ++_it)
	{
		Schema_Object * kvpair = Schema_AddObject(fields_objects, 6);
		::improbable::utils::AddString(kvpair, SCHEMA_MAP_KEY_FIELD_ID, _it->first);
		Schema_AddDouble(kvpair, SCHEMA_MAP_VALUE_FIELD_ID, _it->second);
	}

	// serializing field test_bytes = 7
	::improbable::utils::AddBytes(fields_objects, 7, _test_bytes);

}

TestComponent TestComponent::deserialize(Schema_ComponentData* component_data)
{
	Schema_Object* fields_objects = Schema_GetComponentDataFields(component_data);

	TestComponent data;

	// deserializing field test_int = 1
	data._test_int = Schema_GetInt32(fields_objects, 1);

	// deserializing field test_float = 2
	data._test_float = Schema_GetFloat(fields_objects, 2);

	// deserializing field test_bool = 3
	data._test_bool = Schema_GetBool(fields_objects, 3);

	// deserializing field test_double = 4
	data._test_double = Schema_GetDouble(fields_objects, 4);

	// deserializing field test_list = 5
	{
		uint32_t test_list_length = Schema_GetDoubleCount(fields_objects, 5);
		data._test_list = ::improbable::List<double>(test_list_length);
		Schema_GetDoubleList(fields_objects, 5, data._test_list.data());

	}


	// deserializing field test_map = 6
	{
		data._test_map = ::improbable::Map<std::string, double>();
		auto mapEntryCount = Schema_GetObjectCount(fields_objects, 6);
		for (uint32_t i = 0; i < mapEntryCount; ++i)
		{
			Schema_Object* kvpair = Schema_IndexObject(fields_objects, 6, i);
			auto key = ::improbable::utils::GetString(kvpair, SCHEMA_MAP_KEY_FIELD_ID);
			auto value = Schema_GetDouble(kvpair, SCHEMA_MAP_VALUE_FIELD_ID);
			data._test_map[std::move(key)] = std::move(value);
		}

	}


	// deserializing field test_bytes = 7
	data._test_bytes = ::improbable::utils::GetBytes(fields_objects, 7);

	return data;
}


bool TestComponent::Update::operator==(const TestComponent::Update& value) const
{
	return _test_int == value._test_int &&
	_test_float == value._test_float &&
	_test_bool == value._test_bool &&
	_test_double == value._test_double &&
	_test_list == value._test_list &&
	_test_map == value._test_map &&
	_test_bytes == value._test_bytes;
}

bool TestComponent::Update::operator!=(const TestComponent::Update& value) const
{
	return !operator== (value);
}

TestComponent::Update TestComponent::Update::FromInitialData(const TestComponent& data)
{
	TestComponent::Update update;
	update._test_int.emplace(data.get_test_int());
	update._test_float.emplace(data.get_test_float());
	update._test_bool.emplace(data.get_test_bool());
	update._test_double.emplace(data.get_test_double());
	update._test_list.emplace(data.get_test_list());
	update._test_map.emplace(data.get_test_map());
	update._test_bytes.emplace(data.get_test_bytes());
	return update;
}

TestComponent TestComponent::Update::ToInitialData() const
{
	return TestComponent(
		*_test_int,
		*_test_float,
		*_test_bool,
		*_test_double,
		*_test_list,
		*_test_map,
		*_test_bytes);
}

void TestComponent::Update::ApplyTo(TestComponent& data) const
{
	if (_test_int)
	{
		data.set_test_int(*_test_int);
	}
	if (_test_float)
	{
		data.set_test_float(*_test_float);
	}
	if (_test_bool)
	{
		data.set_test_bool(*_test_bool);
	}
	if (_test_double)
	{
		data.set_test_double(*_test_double);
	}
	if (_test_list)
	{
		data.set_test_list(*_test_list);
	}
	if (_test_map)
	{
		data.set_test_map(*_test_map);
	}
	if (_test_bytes)
	{
		data.set_test_bytes(*_test_bytes);
	}
}

const ::improbable::Option<std::int32_t>& TestComponent::Update::get_test_int() const
{
	return _test_int;
}

::improbable::Option<std::int32_t>& TestComponent::Update::get_test_int()
{
	return _test_int;
}

TestComponent::Update& TestComponent::Update::set_test_int(std::int32_t value)
{
	_test_int.emplace(value);
	return *this;
}

const ::improbable::Option<float>& TestComponent::Update::get_test_float() const
{
	return _test_float;
}

::improbable::Option<float>& TestComponent::Update::get_test_float()
{
	return _test_float;
}

TestComponent::Update& TestComponent::Update::set_test_float(float value)
{
	_test_float.emplace(value);
	return *this;
}

const ::improbable::Option<bool>& TestComponent::Update::get_test_bool() const
{
	return _test_bool;
}

::improbable::Option<bool>& TestComponent::Update::get_test_bool()
{
	return _test_bool;
}

TestComponent::Update& TestComponent::Update::set_test_bool(bool value)
{
	_test_bool.emplace(value);
	return *this;
}

const ::improbable::Option<double>& TestComponent::Update::get_test_double() const
{
	return _test_double;
}

::improbable::Option<double>& TestComponent::Update::get_test_double()
{
	return _test_double;
}

TestComponent::Update& TestComponent::Update::set_test_double(double value)
{
	_test_double.emplace(value);
	return *this;
}

const ::improbable::Option<::improbable::List<double>>& TestComponent::Update::get_test_list() const
{
	return _test_list;
}

::improbable::Option<::improbable::List<double>>& TestComponent::Update::get_test_list()
{
	return _test_list;
}

TestComponent::Update& TestComponent::Update::set_test_list(::improbable::List<double> value)
{
	_test_list.emplace(value);
	return *this;
}

const ::improbable::Option<::improbable::Map<std::string, double>>& TestComponent::Update::get_test_map() const
{
	return _test_map;
}

::improbable::Option<::improbable::Map<std::string, double>>& TestComponent::Update::get_test_map()
{
	return _test_map;
}

TestComponent::Update& TestComponent::Update::set_test_map(::improbable::Map<std::string, double> value)
{
	_test_map.emplace(value);
	return *this;
}

const ::improbable::Option<std::vector<std::uint8_t>>& TestComponent::Update::get_test_bytes() const
{
	return _test_bytes;
}

::improbable::Option<std::vector<std::uint8_t>>& TestComponent::Update::get_test_bytes()
{
	return _test_bytes;
}

TestComponent::Update& TestComponent::Update::set_test_bytes(std::vector<std::uint8_t> value)
{
	_test_bytes.emplace(value);
	return *this;
}

const ::improbable::List< ::improbable::test_schema::TestComponentData::EmptyEvent >& TestComponent::Update::get_empty_list() const
{
	return _empty_list;
}

::improbable::List< ::improbable::test_schema::TestComponentData::EmptyEvent >& TestComponent::Update::get_empty_list()
{
	return _empty_list;
}

TestComponent::Update& TestComponent::Update::add_empty(const ::improbable::test_schema::TestComponentData::EmptyEvent& value)
{
	_empty_list.emplace_back(value);
	return *this;
}

const ::improbable::List< ::improbable::test_schema::TestComponentData::ParameterizedEvent >& TestComponent::Update::get_params_list() const
{
	return _params_list;
}

::improbable::List< ::improbable::test_schema::TestComponentData::ParameterizedEvent >& TestComponent::Update::get_params_list()
{
	return _params_list;
}

TestComponent::Update& TestComponent::Update::add_params(const ::improbable::test_schema::TestComponentData::ParameterizedEvent& value)
{
	_params_list.emplace_back(value);
	return *this;
}
void TestComponent::Update::serialize(Schema_ComponentUpdate* component_update) const
{
	Schema_Object* updates_object = Schema_GetComponentUpdateFields(component_update);
	Schema_Object* events_object = Schema_GetComponentUpdateEvents(component_update);

	// serializing field test_int = 1
	if (_test_int.data() != nullptr)
	{
		Schema_AddInt32(updates_object, 1, (*_test_int));
	}

	// serializing field test_float = 2
	if (_test_float.data() != nullptr)
	{
		Schema_AddFloat(updates_object, 2, (*_test_float));
	}

	// serializing field test_bool = 3
	if (_test_bool.data() != nullptr)
	{
		Schema_AddBool(updates_object, 3, static_cast<std::uint8_t>((*_test_bool)));
	}

	// serializing field test_double = 4
	if (_test_double.data() != nullptr)
	{
		Schema_AddDouble(updates_object, 4, (*_test_double));
	}

	// serializing field test_list = 5
	if (_test_list.data() != nullptr)
	{
		if (_test_list.data()->empty())
		{
			Schema_AddComponentUpdateClearedField(component_update, 5);
		}
		else
		{
			for (std::uint32_t i = 0; i < (*_test_list).size(); ++i)
			{
				Schema_AddDouble(updates_object, 5, (*_test_list)[i]);;
			}
		}
	}

	// serializing field test_map = 6
	if (_test_map.data() != nullptr)
	{
		if (_test_map.data()->empty())
		{
			Schema_AddComponentUpdateClearedField(component_update, 6);
		}
		else
		{
			for (auto _it=(*_test_map).begin(); _it!=(*_test_map).end(); ++_it)
			{
				Schema_Object * kvpair = Schema_AddObject(updates_object, 6);
				::improbable::utils::AddString(kvpair, SCHEMA_MAP_KEY_FIELD_ID, _it->first);
				Schema_AddDouble(kvpair, SCHEMA_MAP_VALUE_FIELD_ID, _it->second);
			}
		}
	}

	// serializing field test_bytes = 7
	if (_test_bytes.data() != nullptr)
	{
		::improbable::utils::AddBytes(updates_object, 7, (*_test_bytes));
	}

	// serializing event empty = 1
	for (auto  empty_it = _empty_list.begin();  empty_it != _empty_list.end(); ++ empty_it)
	{
		 empty_it->serialize(Schema_AddObject(events_object, 1));
	}

	// serializing event params = 2
	for (auto  params_it = _params_list.begin();  params_it != _params_list.end(); ++ params_it)
	{
		 params_it->serialize(Schema_AddObject(events_object, 2));
	}

}

TestComponent::Update TestComponent::Update::deserialize(Schema_ComponentUpdate* component_update)
{
	Schema_Object* updates_object = Schema_GetComponentUpdateFields(component_update);
	Schema_Object* events_object = Schema_GetComponentUpdateEvents(component_update);
	auto fields_to_clear = new Schema_FieldId[Schema_GetComponentUpdateClearedFieldCount(component_update)];
	Schema_GetComponentUpdateClearedFieldList(component_update, fields_to_clear);
	std::set<Schema_FieldId> fields_to_clear_set(fields_to_clear, fields_to_clear + sizeof(fields_to_clear) / sizeof(Schema_FieldId));

	TestComponent::Update data;

	// deserializing field test_int = 1
	if (Schema_GetInt32Count(updates_object, 1) > 0)
	{
		data._test_int = Schema_GetInt32(updates_object, 1);
	}

	// deserializing field test_float = 2
	if (Schema_GetFloatCount(updates_object, 2) > 0)
	{
		data._test_float = Schema_GetFloat(updates_object, 2);
	}

	// deserializing field test_bool = 3
	if (Schema_GetBoolCount(updates_object, 3) > 0)
	{
		data._test_bool = Schema_GetBool(updates_object, 3);
	}

	// deserializing field test_double = 4
	if (Schema_GetDoubleCount(updates_object, 4) > 0)
	{
		data._test_double = Schema_GetDouble(updates_object, 4);
	}

	// deserializing field test_list = 5
	if (Schema_GetDoubleCount(updates_object, 5) > 0)
	{
		uint32_t test_list_length = Schema_GetDoubleCount(updates_object, 5);
		data._test_list = ::improbable::List<double>(test_list_length);
		Schema_GetDoubleList(updates_object, 5, (*data._test_list).data());

	}
	else if (fields_to_clear_set.count(5))
	{
		data._test_list = {};
	}

	// deserializing field test_map = 6
	if (Schema_GetObjectCount(updates_object, 6) > 0)
	{
		data._test_map = ::improbable::Map<std::string, double>();
		auto mapEntryCount = Schema_GetObjectCount(updates_object, 6);
		for (uint32_t i = 0; i < mapEntryCount; ++i)
		{
			Schema_Object* kvpair = Schema_IndexObject(updates_object, 6, i);
			auto key = ::improbable::utils::GetString(kvpair, SCHEMA_MAP_KEY_FIELD_ID);
			auto value = Schema_GetDouble(kvpair, SCHEMA_MAP_VALUE_FIELD_ID);
			(*data._test_map)[std::move(key)] = std::move(value);
		}

	}
	else if (fields_to_clear_set.count(6))
	{
		data._test_map = {};
	}

	// deserializing field test_bytes = 7
	if (Schema_GetBytesCount(updates_object, 7) > 0)
	{
		data._test_bytes = ::improbable::utils::GetBytes(updates_object, 7);
	}

	// deserializing event empty = 1
	for (std::uint32_t i = 0; i < Schema_GetObjectCount(events_object, 1); ++i)
	{
		data.add_empty(improbable::test_schema::TestComponentData::EmptyEvent::deserialize(Schema_IndexObject(events_object, 1, i)));
	}

	// deserializing event params = 2
	for (std::uint32_t i = 0; i < Schema_GetObjectCount(events_object, 2); ++i)
	{
		data.add_params(improbable::test_schema::TestComponentData::ParameterizedEvent::deserialize(Schema_IndexObject(events_object, 2, i)));
	}

	return data;
}


} // namespace test_schema
} // namespace improbable
```
