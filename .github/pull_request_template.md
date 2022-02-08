<!--- Provide a general summary of your changes in the Title above -->

<!--- If this PR is still a Work in Progress [WIP], please open it as DRAFT -->
<!--- Please consider if any label should be added to this PR -->
<!--- If no code has been changed, please add `skip-ci` label -->
<!--- If opening the PR as Draft, please consider adding `no-test` label to only build the code but not run CI -->
<!--- If documentation PR is still pending, please add `doc-pending` label -->

## Description
<!--- Describe changes in detail -->
<!--- If several features/bug fixes are included with these changes, please consider opening separated pull requests -->

<!--- In case of bug fixes, please provide the list of supported branches where this fix should be also merged -->
<!--- Please uncomment following line with the corresponding branches -->
<!--- @Mergifyio backport (branch/es) -->

<!--- If an issue is already opened, please uncomment next line with the corresponding issue number -->
<!--- Fixes # (issue) -->

<!--- In case the changes are built over a previous pull request, please uncomment next line -->
<!--- This PR depends on # (PR) and must be merged after that one. -->

## Contributor Checklist
- [ ] Commit messages follow the project guidelines.
<!--- External contributors should sign the DCO. Fast DDS developers must also refer to the internal Redmine task -->
- [ ] The code follows the style guidelines of this project.
<!--- Please refer to the [Quality Declaration](https://github.com/eProsima/Fast-DDS/blob/master/QUALITY.md#linters-and-static-analysis-4v) for more information -->
- [ ] Tests that thoroughly check the new feature has been added/Regression tests checking the bug and its fix have been added.
<!--- Blackbox tests checking the new functionality are required -->
<!--- Changes that add/modify public API must include unit tests covering all possible cases -->
<!--- In case that no tests are provided, please justify why -->
- [ ] Any new/modified methods have been properly documented using Doxygen (if applicable).

- [ ] Fast DDS test suite has been run locally.
<!--- Please provide the platform/architecture where the test suite has been run -->
<!--- In case that only some tests are run, please provide the list (unit test, blackbox Fast DDS PIM API, blackbox FastRTPS API, etc) -->
- [ ] Changes are ABI compatible.
<!--- Bug fixes should be ABI compatible if possible so a backport to previous affected releases can be made -->
- [ ] Changes are API compatible.
<!--- Public API must not be broken within the same major release -->
- [ ] New feature has been documented/Current behavior is correctly described in the documentation.
<!--- Please uncomment following line with the corresponding PR to the documentation project -->
<!--- Related documentation PR: eProsima/Fast-DDS-docs# (PR) -->
- [ ] Documentation build and tests pass locally.
<!--- Check there are no typos in the Doxygen documentation -->
- [ ] New feature has been added to the `versions.md` file (if applicable).


## Reviewer Checklist
- [ ] Check contributor checklist is correct.

- [ ] Check CI results.
