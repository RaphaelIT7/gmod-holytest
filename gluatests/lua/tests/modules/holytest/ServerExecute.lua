return {
    groupName = "ServerExecute",
    cases = {
        {
            name = "Function exists in holytest table",
            when = HolyTest_IsModuleEnabled( "holytest" ),
            func = function()
                expect( holytest.ServerExecute ).to.beA( "function" )
            end
        },
        {
            name = "holytest table doesn't exist",
            when = not HolyTest_IsModuleEnabled( "holytest" ),
            func = function()
                expect( holytest ).to.beA( "nil" )
            end
        },
        {
            name = "Executes commands properly",
            when = HolyTest_IsModuleEnabled( "holytest" ),
            func = function()
                local called = false
                concommand.Add( "holytest_serverexecutetest", function()
                	called = true
                end )

                RunConsoleCommand( "holytest_serverexecutetest" )
                holytest.ServerExecute()

                expect( called ).to.beTrue()
            end
        },
    }
}
