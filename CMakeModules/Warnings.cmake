

if( MSVC )
   
    #WARNINGS from here..
  #  # EHsc    /EH (Exception Handling Model) - sc - catches C++ exceptions only and tells the compiler to assume
  #  #		  that functions declared as extern "C" never throw a C++ exception.
  #  # C4127 - conditional expression is constant
  #  # C4714 - marked as __forceinline not inlined (I failed to deactivate it selectively)
  #  #         We can disable this warning in the unit tests since it is clear that it occurs
  #  #         because we are oftentimes returning objects that have a destructor or may
  #  #         throw exceptions - in particular in the unit tests we are throwing extra many
  #  #         exceptions to cover indexing errors.
  #  # C4505 - unreferenced local function has been removed (impossible to deactive selectively)
  #  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /wd4127 /wd4505 /wd4714" )
  #
  #  # C4201 - nameless struct/union
  #  # C4100 - unreferenced formal parameter
  #  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4100 /wd4201" )
  #
  #  # C4512 - assignment operator could not be generated
  #  # C4702 - unreachable code
  #  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4512 /wd4702" )
  #
  #  # C4068 - unknown pragma
  #  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4068" )
  #
  #  # C4006 - aldready defined in xxx.obj Second definition ignored
  #  #set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4006" )
    
    option(WARNINGS-SUPPRESS_C4127_conditional_expression_is_constant "conditional expression is constant" ON)
    option(WARNINGS-SUPPRESS_C4505_unreferenced_local_function_has_been_removed "unreferenced local function has been removed (impossible to deactive selectively)" ON)
    option(WARNINGS-SUPPRESS_C4714_function_marked_as___forceinline_not_inlined "marked as __forceinline not inlined (I failed to deactivate it selectively)
            We can disable this warning in the unit tests since it is clear that it occurs
            because we are oftentimes returning objects that have a destructor or may
            throw exceptions - in particular in the unit tests we are throwing extra many
            exceptions to cover indexing errors." ON)

       
    option(WARNINGS-SUPPRESS_C4100_unreferenced_formal_parameter "unreferenced formal parameter" ON)
    option(WARNINGS-SUPPRESS_C4201_nameless_struct_or_union "nameless struct/union" ON)
    option(WARNINGS-SUPPRESS_C4512_assignment_operator_could_not_be_generated "assignment operator could not be generated" ON)
    option(WARNINGS-SUPPRESS_C4702_unreachable_code "unreachable code" ON)   
    option(WARNINGS-SUPPRESS_C4068_unknown_pragma "unknown pragma" ON)
    option(WARNINGS-SUPPRESS_C4006_already_defined_in_xxx_obj_Second_definition_ignored "already defined in xxx.obj. Second definition ignored" ON)
            
    #WARNINGS from BSP
    # disabled
# 4005    'identifier' : macro redefinition
# 4099    PDB 'filename' was not found with 'object/library' or at 'path'; linking object as if no debug info
# 4481    nonstandard extension used: override specifier 'keyword'
# 4189    local variable is initialized but not referenced
# 4996    This function or variable may be unsafe. Consider using...
# treat as errors
# 4552    operator has no effect; expected operator with side-effect
# 4701    potentially uninitialized local variable used
# 4715    not all control paths return a value
# 4716    function must return a value
  #target_compile_options(${TRIDIM_CURRENT_TARGET} PRIVATE /wd4005 /wd4099 /wd4481 /wd4189 /wd4996 /we4552 /we4701 /we4715 /we4716 ) 

    option(WARNINGS-SUPPRESS_C4005_macro_redefinition "'identifier': macro redefinition" ON)
    option(WARNINGS-SUPPRESS_C4099_PDB_was_not_found_with_object_or_library "PDB 'filename' was not found with 'object/library' or at 'path'; linking object as if no debug info" ON)
    option(WARNINGS-SUPPRESS_C4481_nonstandard_extension_used_overrides_specifier "nonstandard extension used: override specifier 'keyword'" ON)
    option(WARNINGS-SUPPRESS_C4189_local_variable_is_initialized_but_not_referenced "local variable is initialized but not referenced" ON)
    option(WARNINGS-SUPPRESS_C4996_This_function_or_variable_may_be_unsafe "This function or variable may be unsafe. Consider using..." ON)
    
    #WARNINGS TO ERRORS
    option(WARNINGS-AS-ERRORS_C4552_operator_has_no_effect "operator has no effect; expected operator with side-effect" ON)
    option(WARNINGS-AS-ERRORS_C4701_potentially_uninitialized_local_variable_used "potentially uninitialized local variable used" ON)
    option(WARNINGS-AS-ERRORS_C4715_not_all_control_paths_return_a_value "not all control paths return a value" ON)
    option(WARNINGS-AS-ERRORS_C4716_function_must_return_a_value "function must return a value" ON)

    
     set(ALL_SUPRESS_WARNINGS
        WARNINGS-SUPPRESS_C4127_conditional_expression_is_constant
        WARNINGS-SUPPRESS_C4505_unreferenced_local_function_has_been_removed
        WARNINGS-SUPPRESS_C4714_function_marked_as___forceinline_not_inlined
        WARNINGS-SUPPRESS_C4100_unreferenced_formal_parameter
        WARNINGS-SUPPRESS_C4201_nameless_struct_or_union
        WARNINGS-SUPPRESS_C4512_assignment_operator_could_not_be_generated
        WARNINGS-SUPPRESS_C4702_unreachable_code
        WARNINGS-SUPPRESS_C4068_unknown_pragma
        WARNINGS-SUPPRESS_C4006_already_defined_in_xxx_obj_Second_definition_ignored
        WARNINGS-SUPPRESS_C4005_macro_redefinition
        WARNINGS-SUPPRESS_C4099_PDB_was_not_found_with_object_or_library
        WARNINGS-SUPPRESS_C4481_nonstandard_extension_used_overrides_specifier
        WARNINGS-SUPPRESS_C4189_local_variable_is_initialized_but_not_referenced
        WARNINGS-SUPPRESS_C4996_This_function_or_variable_may_be_unsafe
    )
    
    set(ALL_ESCALATE_WARNINGS
        WARNINGS-AS-ERRORS_C4552_operator_has_no_effect
        WARNINGS-AS-ERRORS_C4701_potentially_uninitialized_local_variable_used
        WARNINGS-AS-ERRORS_C4715_not_all_control_paths_return_a_value
        WARNINGS-AS-ERRORS_C4716_function_must_return_a_value
    )
    
    #Apply the selected modifications
    foreach(item ${ALL_SUPRESS_WARNINGS})
        
        if(${item})
                        
            #18 is the length of prefix "WARNINGS_SUPRESS_C"
            string(SUBSTRING ${item} 19 4 warning_code)

            add_compile_options("/wd${warning_code}")
            
        endif()
    endforeach()
    
    foreach(item : ${ALL_ESCALATE_WARNINGS})
        
        if(${item})
                        
            #18 is the length of prefix "WARNINGS_ESCALATE_C"
            string(SUBSTRING ${item} 20 4 warning_code)

            add_compile_options("/we${warning_code}")
            
        endif()
    endforeach()    

    # replace all /Wx by /W4
    string( REGEX REPLACE "/W[0-9]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
        
endif()