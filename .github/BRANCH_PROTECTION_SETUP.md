# Branch Protection Setup Guide

This guide provides step-by-step instructions for the repository owner to configure branch protection rules.

## Prerequisites

- You must be the repository owner or have admin access
- Access to GitHub repository settings

## Setup Instructions

### Step 1: Add Collaborators

1. Navigate to your repository: https://github.com/viteacreator/First_prj_rp_picow
2. Click **Settings** tab
3. In the left sidebar, click **Collaborators and teams**
4. Click **Add people** button
5. Enter your colleague's GitHub username or email
6. Choose access level: **Write** (recommended for contributors)
7. Click **Add [username] to this repository**

The collaborator will receive an email invitation and must accept it.

### Step 2: Configure Branch Protection Rules

1. In **Settings**, click **Branches** in the left sidebar
2. Under **Branch protection rules**, click **Add rule** or **Add branch protection rule**
3. In **Branch name pattern**, enter: `main`
4. Configure the following settings:

#### Required Settings:

✅ **Require a pull request before merging**
   - Check this box
   - Optionally: Check "Require approvals" and set number of reviewers (e.g., 1)
   - Optionally: Check "Dismiss stale pull request approvals when new commits are pushed"

✅ **Require status checks to pass before merging** (if you have CI/CD)
   - Check this box
   - Select required status checks

#### Recommended Settings:

✅ **Require conversation resolution before merging**
   - Ensures all review comments are resolved

✅ **Include administrators**
   - Applies rules to repository admins as well
   - **Highly recommended** to ensure consistency

✅ **Restrict who can push to matching branches** (Optional)
   - If you want to completely block direct pushes
   - Leave empty or add specific users/teams

✅ **Do not allow bypassing the above settings**
   - Prevents anyone from bypassing these rules

5. Click **Create** or **Save changes**

### Step 3: Verify Configuration

Test the branch protection by attempting a direct push to main:

```bash
git checkout main
echo "test" >> test.txt
git add test.txt
git commit -m "test direct push"
git push origin main
```

You should receive an error like:
```
remote: error: GH006: Protected branch update failed
remote: error: At least 1 approving review is required by reviewers with write access
```

This confirms branch protection is working correctly.

### Step 4: Inform Your Collaborators

Share these resources with your collaborators:
- [COLABORARE.md](../COLABORARE.md) - Detailed guide in Romanian
- [CONTRIBUTING.md](../CONTRIBUTING.md) - Detailed guide in English
- [README.md](../README.md) - Project overview with quick reference

## Additional Configuration (Optional)

### Enable Required Reviews from Code Owners

If you want to ensure specific people review certain files:

1. The [CODEOWNERS](./CODEOWNERS) file is already created
2. In branch protection settings, enable:
   - ✅ **Require review from Code Owners**
   
This ensures that changes to files require approval from the designated owners.

### Add Status Checks (CI/CD)

If you add GitHub Actions or other CI/CD:

1. Configure your workflow to run on pull requests
2. In branch protection settings:
   - ✅ **Require status checks to pass before merging**
   - Select the status checks that must pass

### Automatically Delete Head Branches

To keep the repository clean:

1. In **Settings** → **General**
2. Under **Pull Requests**
3. ✅ **Automatically delete head branches**

This deletes feature branches after PRs are merged.

## Troubleshooting

### Collaborator Can't Push to Any Branch

**Problem**: Collaborator gets "Permission denied" error for all branches

**Solutions**:
1. Verify the collaborator accepted the invitation (check their email)
2. Ensure they have at least "Write" access (Settings → Collaborators)
3. Check if they're using the correct authentication (HTTPS with token, or SSH)

### Collaborator Can Push to Main

**Problem**: Branch protection doesn't seem to work

**Solutions**:
1. Verify branch protection rule is created for branch pattern `main`
2. Check if "Do not allow bypassing" is enabled
3. Ensure the rule is active (there's a green checkmark next to it)

### Can't Create Pull Request

**Problem**: No option to create PR after pushing branch

**Solutions**:
1. Ensure the branch was pushed successfully: `git push -u origin branch-name`
2. Refresh the GitHub repository page
3. Navigate to "Pull requests" tab and click "New pull request"

## Best Practices

1. **Always use Pull Requests**: Even as the owner, use PRs for your changes to maintain code quality
2. **Review Promptly**: Respond to PRs quickly to keep collaborators productive
3. **Communicate Changes**: Use PR descriptions and comments to explain changes clearly
4. **Keep Main Stable**: Only merge tested, working code into main
5. **Regular Updates**: Encourage collaborators to sync with main frequently

## Support

For more information:
- [GitHub Docs: Protected Branches](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-protected-branches)
- [GitHub Docs: Managing Access](https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/managing-repository-settings/managing-teams-and-people-with-access-to-your-repository)

If you encounter issues not covered here, open an issue in this repository.
